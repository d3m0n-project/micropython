#include "py/builtin.h"
#include "py/compile.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "memory.h"
#include "stdlib.h"
#include "stdio.hpp"

int main(int argc, char **argv)
{
	char *heap = (char *)malloc(MICROPY_HEAP_SIZE);
	if (heap == NULL) {
		printf("Micropython: unable to allocate %u bytes for GC heap\n", (unsigned)MICROPY_HEAP_SIZE);
		return 1;
	}

	mp_cstack_init_with_sp_here(40 * 1024);
	gc_init(heap, heap + MICROPY_HEAP_SIZE);
	mp_init();

	// arg parser
	if (argc == 2)
	{
		if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1]))
		{
			printf("Micropython fork for d3m0n os: help\n");
			printf("	--help -h		get this message\n");
			printf("	[path/file.py]   execute directly python code\n");
			return 0;
		}
		if (!strncmp("-", argv[1], 1)) // another arg
		{
			fprintf(stderr, "\033[31mERR\033[0m: Usage: %s [path/file.py] [-h --help]\n", argv[0]);
			return 1;
		}
		
		// start a file
		int ret = pyexec_file(argv[1]);
		return (ret != PYEXEC_NORMAL_EXIT);
	}

	// start repl
	pyexec_friendly_repl();

	// deinit the runtime.
	gc_sweep_all();
	mp_deinit();
	return 0;
}

// uncaught exceptions
void	nlr_jump_fail(void *val)
{
	for (;;)
		continue;
}

void	gc_collect(void)
{
	gc_collect_start();
	gc_helper_collect_regs_and_stack();
	gc_collect_end();
}


static bool import_path_join(char *dest, size_t dest_len, const char *prefix, const char *path)
{
	size_t prefix_len = strlen(prefix);
	size_t path_len = strlen(path);
	if (prefix_len != 0 && prefix[prefix_len - 1] == '/' && path[0] == '/') {
		++path;
		--path_len;
	}
	if (prefix_len + path_len + 1 > dest_len)
		return false;
	memcpy(dest, prefix, prefix_len);
	memcpy(dest + prefix_len, path, path_len + 1);
	return true;
}

static bool import_file_exists(const char *path)
{
	int fd = open(path, O_READ);
	if (fd < 0)
		return false;
	close(fd);
	return true;
}


static bool make_lib_import_candidate(const char *path, size_t layout, char *resolved, size_t resolved_len)
{
	if (path[0] == '/' || path[0] == '\0' || path[0] == '.')
		return false;

	const char *slash = strchr(path, '/');
	size_t package_len = slash == NULL ? strlen(path) : (size_t)(slash - path);
	if (package_len == 0 || package_len >= 128)
		return false;
	char prefix[160];
	const char *suffixes[] = {"/python/", "/"};
	size_t prefix_len = strlen("/lib/");
	memcpy(prefix, "/lib/", prefix_len);
	memcpy(prefix + prefix_len, path, package_len);
	prefix_len += package_len;
	size_t suffix_len = strlen(suffixes[layout]);
	memcpy(prefix + prefix_len, suffixes[layout], suffix_len + 1);
	const char *tail = slash == NULL ? "" : slash + 1;
	return import_path_join(resolved, resolved_len, prefix, tail);
}

static bool resolve_lib_import(const char *path, char *resolved, size_t resolved_len)
{
	for (size_t layout = 0; layout < 2; ++layout)
    {
		if (make_lib_import_candidate(path, layout, resolved, resolved_len) && resolved[strlen(resolved) - 1] != '/' && import_file_exists(resolved))
			return true;
	}
	return false;
}

mp_import_stat_t mp_import_stat(const char *path)
{
	char resolved[MICROPY_ALLOC_PATH_MAX];
	char init_path[MICROPY_ALLOC_PATH_MAX];
	// a package is when its __init__.py exists in dir
	for (size_t layout = 0; layout < 2; ++layout)
    {
		if (make_lib_import_candidate(path, layout, resolved, sizeof(resolved)) && import_path_join(init_path, sizeof(init_path), resolved, "/__init__.py") && import_file_exists(init_path))
            return MP_IMPORT_STAT_DIR;
    }
	if (resolve_lib_import(path, resolved, sizeof(resolved)))
		return MP_IMPORT_STAT_FILE;

	if (import_file_exists(path))
		return MP_IMPORT_STAT_FILE;

    return MP_IMPORT_STAT_NO_EXIST;
}

mp_lexer_t  *mp_lexer_new_from_file(qstr filename)
{
	const char *path = qstr_str(filename);
	mp_reader_t reader;
	char resolved[MICROPY_ALLOC_PATH_MAX];
	if (resolve_lib_import(path, resolved, sizeof(resolved)))
		path = resolved;
	mp_reader_new_file(&reader, qstr_from_str(path));
	//if (err != 0)
	//	mp_raise_OSError(err);
	return mp_lexer_new(filename, reader);
}
