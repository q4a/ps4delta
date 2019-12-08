
// Copyright (C) Force67 2019

#include "elfLoader.h"
#include "vmodLinker.h"
#include "kernel/Process.h"

#include <utl/File.h>

namespace modules
{
	elfLoader::elfLoader(std::unique_ptr<uint8_t[]> d) :
		data(std::move(d)),
		tlsslot(-1)
	{}

	bool elfLoader::canLoad(utl::File& file)
	{
		ELFHeader elf{};
		file.Read(elf);

		if (elf.magic == ELF_MAGIC && elf.machine == ELF_MACHINE_X86_64) {
			LOG_TRACE("Identified file as plain ELF");
			return true;
		}

		return false;
	}

	bool elfLoader::loadinProc(krnl::Process& proc)
	{
		elf = GetOffset<ELFHeader>(0);
		segments = GetOffset<ELFPgHeader>(elf->phoff);

		for (uint16_t i = 0; i < elf->phnum; i++) {

			auto s = &segments[i];
			LOG_TRACE("SEGMENT {}, {},{} -> TYPE {}, size {}", i, s->offset, s->vaddr, SegTypeToString(s->type), s->filesz);

			switch (s->type) {
			case PT_LOAD:
			{
				// test alignment
				if (s->align & 0x3FFFull || s->vaddr & 0x3FFFull || s->offset & 0x3FFFull) {
					LOG_ERROR("elfLoader: Bad section alignment");
					return false;
				}

				LOG_ASSERT(s->filesz > s->memsz);
				break;
			}
			case PT_TLS:
			{
				LOG_ASSERT(s->filesz > s->memsz);
		
				if (s->align > 32) {
					LOG_ERROR("elfLoader: Bad TLS seg alignment");
					return false;
				}

				break;
			}
			case PT_SCE_DYNLIBDATA:
			{
				LOG_ASSERT(s->filesz == 0);

				dynld.ptr = GetOffset<char>(s->offset);
				dynld.size = s->filesz;
				break;
			}
			case PT_DYNAMIC:
			{
				LOG_ASSERT(s->filesz > s->memsz);
				break;
			}
			}
		}

		doDynamics();
		logDbgInfo();

		if (!MapImage(proc) ||
			!SetupTLS(proc)) {
			LOG_ERROR("elfLoader: Failed to map image");
			return false;
		}

		if (!ProcessRelocations()) {
			LOG_ERROR("elfLoader: Failed to relocate image");
			return false;
		}

		if (!ResolveImports()) {
			LOG_ERROR("elfLoader: Failed to resolve imports");
			return false;
		}

		installEHFrame();

		// and register the entry...
		auto entry = std::make_shared<krnl::Module>();
		entry->name = "#MODULE#";
		entry->base = targetbase;
		entry->sizeCode = totalimage;
		entry->tlsSlot = tlsslot;

		if (elf->entry == 0)
			entry->entry = nullptr;
		else
			entry->entry = targetbase + elf->entry;

#if 0
		{
			utl::File proc("lastElf.bin", utl::fileMode::write);
			if (proc.IsOpen()) {
				proc.Write(entry->base, entry->sizeCode);
			}
		}
#endif

		proc.RegisterModule(std::move(entry));
		return true;
	}

	void elfLoader::doDynamics()
	{
		auto* s = GetSegment(ElfSegType::PT_DYNAMIC);
		ELFDyn* dynamics = GetOffset<ELFDyn>(s->offset);
		for (int32_t i = 0; i < (s->filesz / sizeof(ELFDyn)); i++) {
			auto* d = &dynamics[i];

			switch (d->tag) {
			case DT_INIT:
			case DT_INIT_ARRAY:
			{
				LOG_TRACE("DT_INIT {}", d->un.value);
				break;
			}
			case DT_SCE_JMPREL:
			{
				if (d->un.ptr > dynld.size) {
					std::printf("[!] bad JMPREL offset: %llx\n", d->un.ptr);
					continue;
				}

				jmpslots = (ElfRel*)(dynld.ptr + d->un.ptr);
				break;
			}
			case DT_PLTRELSZ:
			case DT_SCE_PLTRELSZ:
			{
				numJmpSlots = static_cast<uint32_t>(d->un.value / sizeof(ElfRel));
				break;
			}
			case DT_SCE_STRTAB:
			{
				if (d->un.ptr > dynld.size) {
					std::printf("[!] bad STRTAB offset: %llx", d->un.ptr);
					continue;
				}

				strtab.ptr = (char*)(dynld.ptr + d->un.ptr);
				break;
			}
			case DT_STRSZ:
			case DT_SCE_STRSIZE:
			{
				strtab.size = d->un.value;
				break;
			}
			case DT_SCE_EXPLIB:
			case DT_SCE_IMPLIB:
			{

				break;
			}
			case DT_SCE_SYMTAB:
			{
				if (d->un.ptr > dynld.size) {
					std::printf("[!] bad SYMTAB offset: %llx", d->un.ptr);
					continue;
				}

				symbols = (ElfSym*)(dynld.ptr + d->un.ptr);
				break;
			}
			case DT_SCE_SYMTABSZ:
			{
				numSymbols = static_cast<uint32_t>(d->un.value / sizeof(ElfSym));
				break;
			}
			case DT_SCE_RELA:
			{
				rela = (ElfRel*)(dynld.ptr + d->un.ptr);
				break;
			}
			case DT_RELASZ:
			case DT_SCE_RELASZ:
			{
				numRela = static_cast<uint32_t>(d->un.value / sizeof(ElfRel));
				break;
			}
			}
		}

		for (int32_t i = 0; i < (s->filesz / sizeof(ELFDyn)); i++) {
			auto* d = &dynamics[i];
			switch (d->tag) {
				/*case DT_NEEDED:
				{
					std::printf("%i: DT_NEEDED %s\n", i, (char*)(strtab.ptr + d->un.value));
					break;
				}*/
				case DT_SCE_NEEDED_MODULE:
				{
					auto& e = implibs.emplace_back();
					e.name = (const char*)(strtab.ptr + (d->un.value & 0xFFFFFFFF));
					e.modid = d->un.value >> 48;

				//	std::printf("%s\n", e.name);
					break;
				}
			}
		}
	}

	bool elfLoader::MapImage(krnl::Process &proc)
	{
	/*	auto* it = GetSegment(PT_SCE_RELRO);
		if (it) {
			for (int i = 0; i < it->filesz; i += 8) {
				std::printf("RELRO entry %llx\n", *GetOffset<uintptr_t>(it->offset + i));
			}
		}*/

		// the stuff we actually care about
		totalimage = 0;
		for (uint16_t i = 0; i < elf->phnum; ++i) {
			const auto* p = &segments[i];
			if (p->type == PT_LOAD) {
				totalimage += (p->memsz + 0xFFF) & ~0xFFF;
			}
		}

		// could also check if INTERP exists
		if (totalimage == 0)
			return false;

		// reserve segment
		targetbase = (uint8_t*)proc.GetVirtualMemory().AllocateSeg(totalimage);
		if (!targetbase)
			return false;

		// and move it!
		for (uint16_t i = 0; i < elf->phnum; i++) {
			auto s = &segments[i];
			if (s->type == PT_LOAD) {
				uint32_t perm = s->flags & (PF_R | PF_W | PF_X);
				switch (perm) {

				// todo: module segment collection :D
				case (PF_R | PF_X):
				{
					// its a codepage
					std::memcpy(GetAddress<void>(s->vaddr), GetOffset<void>(s->offset), s->filesz);
					break;
				}
				case (PF_R):
				{
					//Reserve a rdata segment
					std::memcpy(GetAddress<void>(s->vaddr), GetOffset<void>(s->offset), s->filesz);
					break;
				}
				case (PF_R | PF_W):
				{
					// reserve a read write data seg
					std::memcpy(GetAddress<void>(s->vaddr), GetOffset<void>(s->offset), s->filesz);
					break;
				}
				default:
				{
					std::puts("UNKNOWN param!");

				}
				}
			}
			else if (s->type == PT_SCE_RELRO) {
				std::memcpy(GetAddress<void>(s->vaddr), GetOffset<void>(s->offset), s->filesz);
			}
		}

		return true;
	}

	bool elfLoader::SetupTLS(krnl::Process& proc)
	{
		auto* p = GetSegment(PT_TLS);
		if (p) {

			tlsslot = proc.GetNextFreeTls();
		}

		return true;
	}

	// pltrela_table_offset 
	bool elfLoader::ResolveImports()
	{
		for (uint32_t i = 0; i < numJmpSlots; i++) {
			auto* r = &jmpslots[i];

			int32_t type = ELF64_R_TYPE(r->info);
			int32_t isym = ELF64_R_SYM(r->info);

			ElfSym* sym = &symbols[isym];

			if (type != R_X86_64_JUMP_SLOT) {
				LOG_WARNING("resolveImports: bad jump slot {}", i);
				continue;;
			}

			if ((uint32_t)isym >= numSymbols ||
				sym->st_name >= strtab.size) {
				LOG_WARNING("resolveImports: bad symbol index {} for relocation {}", isym, i);
				continue;
			}

			const char* name = &strtab.ptr[sym->st_name];
			//std::printf("SYMBOL binding %s, BINDING: %d, TYPE: %d\n", name, sym->st_info >> 4, sym->st_info & 0x0f);

			// example: weDug8QD-lE#L#M
			//					    ^ ^
			// see above: libid, modid

			int32_t binding = ELF64_ST_BIND(sym->st_info);
			if (binding == STB_LOCAL) {
				*GetAddress<uintptr_t>(r->offset) = (uintptr_t)GetAddress<uintptr_t>(sym->st_value);
				break;
			}

			if (std::strlen(name) == 15) {

				auto *ptr = &name[14];

				uint64_t modid = 0;
				modules::decodeNid(ptr, 1, modid);

				//char encoded[12]{};
				//strncpy(encoded, name, 11);
		
				// use ps4libdoc to find real name
				uint64_t hid = 0;
				if (!modules::decodeNid(name, 11, hid))
					return false;
	
				// fetch the import module name
				for (auto& imp : implibs) {
					if (imp.modid == static_cast<int32_t>(modid)) {

					// ... and set the import address
					*GetAddress<uintptr_t>(r->offset) = modules::getImport(imp.name, hid);
					break;
				}
				}
			} 
		}

		return true;
	}

	bool elfLoader::ProcessRelocations()
	{
		for (size_t i = 0; i < numRela; i++) {
			auto* r = &rela[i];

			uint32_t isym = ELF64_R_SYM(r->info);
			int32_t type = ELF64_R_TYPE(r->info);

			ElfSym* sym = &symbols[isym];

			if (isym >= numSymbols) {
				std::printf("[!] Invalid symbol index %d\n", isym);
				continue;
			}

			switch (type) {
			case R_X86_64_NONE: break;
			case R_X86_64_64:
			{
				// class type info and such..
				//std::printf("RELOCATION SHIT %s\n", (char*)& strtab.ptr[sym->st_name]);
				*GetAddress<uint64_t>(r->offset) = sym->st_value + r->addend;
				break;
			}
			case R_X86_64_RELATIVE: /* base + ofs*/
			{
				*GetAddress<int64_t>(r->offset) = (int64_t)GetAddress<int64_t>(r->addend);
				break;
			}
			case R_X86_64_GLOB_DAT:
			{
				*GetAddress<uint64_t>(r->offset) = sym->st_value;
				break;
			}
			case R_X86_64_PC32:
			{
				*GetAddress<uint32_t>(r->offset) = (uint32_t)(sym->st_value + r->addend - (uint64_t)GetAddress<uint64_t>(r->offset));
				break;
			}
			case R_X86_64_DTPMOD64:
			{
				// set tls index for image
				*GetAddress<uint16_t>(r->offset) = tlsslot;
				break;
			}
			case R_X86_64_TPOFF64:
			case R_X86_64_TPOFF32: 
			default:
			{
				std::printf("unknown relocation type %d\n", type);
				return false;
			}

			}
		}

		return true;
	}

	// taken from idc's "uplift" project
	void elfLoader::installEHFrame()
	{
		const auto* p = GetSegment(PT_GNU_EH_FRAME);
		if (p->filesz > p->memsz)
			return;

		// custom struct for eh_frame_hdr 
		struct GnuExceptionInfo
		{
			uint8_t version;
			uint8_t encoding;
			uint8_t fdeCount;
			uint8_t encodingTable;
			uint8_t first;
		};

		auto* info = GetOffset<GnuExceptionInfo>(p->offset);

		if (info->version != 1)
			return;

		uint8_t* data_buffer = nullptr;
		uint8_t* current = &info->first;

		if (info->encoding == 0x03) // relative to base address
		{
			auto offset = *reinterpret_cast<uint32_t*>(current);
			current += 4;
			data_buffer = (uint8_t*)&targetbase[offset];
		}
		else if (info->encoding == 0x1B) // relative to eh_frame
		{
			auto offset = *reinterpret_cast<int32_t*>(current);
			current += 4;
			data_buffer = &current[offset];
		}
		else
		{
			return;
		}

		if (!data_buffer)
		{
			return;
		}

		uint8_t* data_buffer_end = data_buffer;
		while (true)
		{
			size_t size = *reinterpret_cast<int32_t*>(data_buffer_end);
			if (size == 0)
			{
				data_buffer_end = &data_buffer_end[4];
				break;
			}
			if (size == -1)
			{
				size = 12 + *reinterpret_cast<size_t*>(&data_buffer_end[4]);
			}
			else
			{
				size = 4 + size;
			}
			data_buffer_end = &data_buffer_end[size];
		}

		size_t fde_count;
		if (info->fdeCount == 0x03) // absolute
		{
			fde_count = *reinterpret_cast<uint32_t*>(current);
			current += 4;
		}
		else
		{
			return;
		}

		if (info->encodingTable != 0x3B) // relative to eh_frame
		{
			return;
		}
	}

	void elfLoader::logDbgInfo()
	{
		for (uint16_t i = 0; i < elf->phnum; i++) {
			auto s = &segments[i];
			switch (s->type) {
			case PT_SCE_PROCPARAM:
			{
				struct SCEPROC
				{
					uint32_t length;
					uint32_t unk;
					uint32_t magic;
					uint32_t version;
					uint32_t ofs;
				};

				SCEPROC* pr = GetOffset<SCEPROC>(s->offset);


				//std::printf("FOUND PROCPARAM %llx (%llx byts big) proc ofs %llx, %llx\n", s->offset, s->filesz, pr->ofs, GetOffset<uintptr_t>(pr->ofs));

			} break;

			// like the PDB path on windows
			case PT_SCE_COMMENT:
			{
				auto* comment = GetOffset<SCEComment>(s->offset);

				std::string name;
				name.resize(comment->pathLength);
				memcpy(name.data(), GetOffset<void>(s->offset + sizeof(SCEComment)), comment->pathLength);

				std::printf("Comment %s, %d\n", name.c_str(), comment->unk);
				break;
			}
			case PT_SCE_LIBVERSION:
			{
				uint8_t* sec = GetOffset<uint8_t>(s->offset);

				// count entries
				int32_t index = 0;
				while (index <= s->filesz) {

					int8_t cb = sec[index];

					// skip control byte
					index++;

					for (int i = index; i < (index + cb); i++)
					{
						if (sec[i] == 0x3A) {

							size_t length = i - index;

							std::string name;
							name.resize(length);
							memcpy(name.data(), &sec[index], length);

							uint32_t version = *(uint32_t*)& sec[i + 1];
							uint8_t* vptr = (uint8_t*)& version;

							//std::printf("lib <%s>, version %x.%x.%x.%x\n", name.c_str(), vptr[0], vptr[1], vptr[2], vptr[3]);
							break;
						}
					}

					// skip forward
					index += cb;
				}
				break;
			}
			}
		}
	}
}