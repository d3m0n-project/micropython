#include "py/builtin.h"
#include "py/compile.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "stdlib.h"
#include "stdio.hpp"

static char heap[4096];

int main(int argc, char **argv)
{
	// Initialise the MicroPython runtime.
	mp_cstack_init_with_sp_here(2048);
	gc_init(heap, heap + sizeof(heap));
	mp_init();

    // arg parser
    if (argc == 2)
	{
        if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1]))
        {
            printf("Micropython fork for d3m0n os: help\n");
            printf("    --help -h        get this message\n");
            printf("    [path/file.py]   execute directly python code\n");
            return 0;
        }
        if (!strncmp("-", argv[1], 1)) // another arg
        {
            fprintf(stderr, "\033[31mERR\033[0m: Usage: %s [path/file.py] [-h --help]\n", argv[0]);
            return 1;
        }
        
        // start a file
        int ret = pyexec_file(argv[1]);
        return ret;
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
    {
        continue;
	}
}

void    gc_collect(void)
{
	gc_collect_start();
	gc_helper_collect_regs_and_stack();
	gc_collect_end();
}

mp_import_stat_t    mp_import_stat(const char *path)
{
	return MP_IMPORT_STAT_NO_EXIST;
}

mp_lexer_t  *mp_lexer_new_from_file(qstr filename)
{
    fprintf(stderr, "\033[31mERR\033[0m: Could not find %s\n", qstr_str(filename));
    exit(1);
	//mp_raise_OSError(MP_ENOENT);
}