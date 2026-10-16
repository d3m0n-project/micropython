

#include "types.h"
#include "py/runtime.h"
#include "py/objstr.h"
#include "py/objtuple.h"
#include "sys.h"

static void sys_check_error(int result)
{
	if (result < 0) {
		mp_raise_OSError(-result);
	}
}

static const char *sys_get_path(mp_obj_t obj)
{
	return mp_obj_str_get_str(obj);
}

static mp_int_t sys_get_int(mp_obj_t obj)
{
	return mp_obj_get_int(obj);
}

/* argv */
static mp_obj_t sys_argv(void)
{
	// TODO:
	return mp_obj_new_list(0, NULL);
}

static MP_DEFINE_CONST_FUN_OBJ_0(sys_argv_obj, sys_argv);

/* exit(code) */
static mp_obj_t sys_exit(mp_obj_t code_obj)
{
	int code = sys_get_int(code_obj);
	exit(code);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_exit_obj, sys_exit);

/* read(fd, count) -> bytes */
static mp_obj_t sys_read(size_t n_args, const mp_obj_t *args)
{
	(void)n_args;
	int fd = sys_get_int(args[0]);
	mp_int_t count = sys_get_int(args[1]);

	if (count < 0) {
		mp_raise_ValueError(MP_ERROR_TEXT("negative count"));
	}

	vstr_t buffer;
	vstr_init_len(&buffer, (size_t)count);

	int result = read(fd, buffer.buf, (uint32_t)count);

	if (result < 0) {
		vstr_clear(&buffer);
		sys_check_error(result);
	}

	if (result > count) {
		vstr_clear(&buffer);
		mp_raise_ValueError(MP_ERROR_TEXT("invalid read result"));
	}

	mp_obj_t out = mp_obj_new_bytes(
		(const byte *)buffer.buf,
		(size_t)result
	);

	vstr_clear(&buffer);
	return out;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(
	sys_read_obj, 2, 2, sys_read
);

/* write(fd, data) -> number of bytes written */
static mp_obj_t sys_write(mp_obj_t fd_obj, mp_obj_t data_obj)
{
	int fd = sys_get_int(fd_obj);
	mp_buffer_info_t buffer;

	mp_get_buffer_raise(data_obj, &buffer, MP_BUFFER_READ);

	int result = write(
		fd,
		(const char *)buffer.buf,
		(uint32_t)buffer.len
	);

	sys_check_error(result);
	return mp_obj_new_int(result);
}

static MP_DEFINE_CONST_FUN_OBJ_2(sys_write_obj, sys_write);

/* open(path, flags) -> fd */
static mp_obj_t sys_open(mp_obj_t path_obj, mp_obj_t flags_obj)
{
	const char *path = sys_get_path(path_obj);
	uint32_t flags = (uint32_t)mp_obj_get_int_truncated(flags_obj);

	int fd = open(path, flags);
	sys_check_error(fd);

	return mp_obj_new_int(fd);
}

static MP_DEFINE_CONST_FUN_OBJ_2(sys_open_obj, sys_open);

/* close(fd) */
static mp_obj_t sys_close(mp_obj_t fd_obj)
{
	int result = close(sys_get_int(fd_obj));
	sys_check_error(result);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_close_obj, sys_close);

/* lseek(fd, offset, whence) */
static mp_obj_t sys_lseek(mp_obj_t fd_obj, mp_obj_t offset_obj, mp_obj_t whence_obj)
{
	int fd = sys_get_int(fd_obj);
	size_t offset = (size_t)mp_obj_get_int_truncated(offset_obj);
	e_seek_directive whence =
		(e_seek_directive)mp_obj_get_int(whence_obj);

	size_t result = lseek(fd, offset, whence);

	return mp_obj_new_int_from_uint((mp_uint_t)result);
}

static MP_DEFINE_CONST_FUN_OBJ_3(sys_lseek_obj, sys_lseek);

/* uname(buffer_len) -> str */
static mp_obj_t sys_uname(mp_obj_t length_obj)
{
	mp_int_t length = sys_get_int(length_obj);

	if (length <= 0)
		mp_raise_ValueError(MP_ERROR_TEXT("invalid buffer length"));

	vstr_t buffer;
	vstr_init_len(&buffer, (size_t)length);
	memset(buffer.buf, 0, (size_t)length);

	int result = uname(buffer.buf, (uint32_t)length);
	if (result < 0)
	{
		vstr_clear(&buffer);
		sys_check_error(result);
	}

	size_t actual_len = 0;
	while (actual_len < (size_t)length && buffer.buf[actual_len] != '\0')
		actual_len++;

	mp_obj_t out = mp_obj_new_str(buffer.buf, actual_len);
	vstr_clear(&buffer);
	return out;
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_uname_obj, sys_uname);

/* getdents(path, buffer_len) -> bytes */
static mp_obj_t	sys_getdents(mp_obj_t path_obj, mp_obj_t length_obj)
{
	const char *path = sys_get_path(path_obj);
	mp_int_t length = sys_get_int(length_obj);

	if (length < 0)
		mp_raise_ValueError(MP_ERROR_TEXT("negative buffer length"));

	vstr_t buffer;
	vstr_init_len(&buffer, (size_t)length);

	int result = getdents(path, buffer.buf, (uint32_t)length);
	if (result < 0)
	{
		vstr_clear(&buffer);
		sys_check_error(result);
	}

	if (result > length)
	{
		vstr_clear(&buffer);
		mp_raise_ValueError(MP_ERROR_TEXT("invalid getdents result"));
	}

	mp_obj_t out = mp_obj_new_bytes(
		(const byte *)buffer.buf,
		(size_t)result
	);

	vstr_clear(&buffer);
	return out;
}

static MP_DEFINE_CONST_FUN_OBJ_2(sys_getdents_obj, sys_getdents);

/* rename(old_path, new_path) */
static mp_obj_t sys_rename(mp_obj_t old_path_obj, mp_obj_t new_path_obj)
{
	int result = rename(
		sys_get_path(old_path_obj),
		sys_get_path(new_path_obj)
	);

	sys_check_error(result);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_2(sys_rename_obj, sys_rename);

/* mkdir(path) */
static mp_obj_t sys_mkdir(mp_obj_t path_obj)
{
	int result = mkdir(sys_get_path(path_obj));
	sys_check_error(result);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_mkdir_obj, sys_mkdir);

/* rmdir(path) */
static mp_obj_t sys_rmdir(mp_obj_t path_obj)
{
	int result = rmdir(sys_get_path(path_obj));
	sys_check_error(result);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_rmdir_obj, sys_rmdir);

/* remove(path) */
static mp_obj_t sys_remove(mp_obj_t path_obj)
{
	int result = remove(sys_get_path(path_obj));
	sys_check_error(result);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_remove_obj, sys_remove);

/* surface_create(width, height) -> (address, pitch, return_code) */
static mp_obj_t	sys_surface_create(mp_obj_t width_obj, mp_obj_t height_obj)
{
    uint32_t width  = (uint32_t)mp_obj_get_int_truncated(width_obj);
    uint32_t height = (uint32_t)mp_obj_get_int_truncated(height_obj);

    uint8_t *surface = NULL;
    int pitch = 0;

    int result = surface_create(width, height, &surface, &pitch);

    mp_obj_t items[3] = {
        mp_obj_new_int_from_uint((uintptr_t)surface),  // ← was mp_obj_new_int + cast
        mp_obj_new_int(pitch),
        mp_obj_new_int(result),
    };
    return mp_obj_new_tuple(3, items);
}

static MP_DEFINE_CONST_FUN_OBJ_2(
	sys_surface_create_obj, sys_surface_create
);

/* c_buffer_uint32_set(address, offset, value) */
static mp_obj_t	sys_c_buffer_uint32_set(mp_obj_t address_obj, mp_obj_t offset_obj, mp_obj_t value_obj)
{
    uintptr_t address = (uintptr_t)mp_obj_get_int_truncated(address_obj);
    mp_int_t signed_offset = mp_obj_get_int(offset_obj);  // error on negative
    if (signed_offset < 0)
        mp_raise_ValueError(MP_ERROR_TEXT("offset must be non-negative"));
	
    uintptr_t offset_count = (uintptr_t)signed_offset;
    uint32_t value = (uint32_t)mp_obj_get_int_truncated(value_obj);
    if (offset_count > UINTPTR_MAX / sizeof(uint32_t))
        mp_raise_ValueError(MP_ERROR_TEXT("offset overflow"));
	
    uintptr_t byte_offset = offset_count * sizeof(uint32_t);
    if (byte_offset > UINTPTR_MAX - address)
        mp_raise_ValueError(MP_ERROR_TEXT("address overflow"));

    uint32_t *ptr = (uint32_t *)(address + byte_offset);
    *ptr = value;   // direct assignment is cleaner than memcpy for aligned uint32
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_3(
	sys_c_buffer_uint32_set_obj, sys_c_buffer_uint32_set
);

/* surface_update(address) */
static mp_obj_t sys_surface_update(mp_obj_t surface_obj)
{
	uintptr_t address = (uintptr_t)mp_obj_int_get_truncated(surface_obj);

	surface_update((uint8_t *)address);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(
	sys_surface_update_obj, sys_surface_update
);

/* sbrk(increment) -> address */
static mp_obj_t sys_sbrk(mp_obj_t increment_obj)
{
	size_t increment =
		(size_t)mp_obj_get_int_truncated(increment_obj);

	void *address = sbrk(increment);

	return mp_obj_new_int_from_uint((uintptr_t)address);
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_sbrk_obj, sys_sbrk);

/* time() */
static mp_obj_t sys_time(void)
{
	return mp_obj_new_int_from_ull(
		(uint64_t)(uintptr_t)time(NULL)
	);
}

static MP_DEFINE_CONST_FUN_OBJ_0(sys_time_obj, sys_time);

/* clock() */
static mp_obj_t sys_clock(void)
{
	return mp_obj_new_int_from_ull((uint64_t)clock());
}

static MP_DEFINE_CONST_FUN_OBJ_0(sys_clock_obj, sys_clock);

/* random_u32() */
static mp_obj_t sys_random_u32(void)
{
	return mp_obj_new_int_from_uint(random_u32());
}

static MP_DEFINE_CONST_FUN_OBJ_0(
	sys_random_u32_obj, sys_random_u32
);

/* exec(path, argv) */
static mp_obj_t sys_exec(mp_obj_t path_obj, mp_obj_t argv_obj)
{
	const char *path = sys_get_path(path_obj);

	if (!mp_obj_is_type(argv_obj, &mp_type_list) &&
		!mp_obj_is_type(argv_obj, &mp_type_tuple)) {
		mp_raise_TypeError(MP_ERROR_TEXT(
			"argv must be a list or tuple"
		));
	}

	size_t argc;
	mp_obj_t *items;

	mp_obj_get_array(argv_obj, &argc, &items);

	const char **argv = m_new(const char *, argc + 1);
	for (size_t i = 0; i < argc; i++)
		argv[i] = mp_obj_str_get_str(items[i]);
	argv[argc] = NULL;

	exec(path, argv);

	m_del(const char *, argv, argc + 1);
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_2(sys_exec_obj, sys_exec);

/* kill(pid, code) */
static mp_obj_t sys_kill(mp_obj_t pid_obj, mp_obj_t code_obj)
{
	uint32_t pid =
		(uint32_t)mp_obj_get_int_truncated(pid_obj);
	int code = sys_get_int(code_obj);

	int result = kill(pid, code);
	sys_check_error(result);

	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_2(sys_kill_obj, sys_kill);

/* pstate(pid) */
static mp_obj_t sys_pstate(mp_obj_t pid_obj)
{
	uint32_t pid = (uint32_t)mp_obj_get_int_truncated(pid_obj);
	int result = pstate(pid);
	sys_check_error(result);
	return mp_obj_new_int(result);
}

static MP_DEFINE_CONST_FUN_OBJ_1(sys_pstate_obj, sys_pstate);

/* Module globals */
static const mp_rom_map_elem_t sys_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sys) },

	{ MP_ROM_QSTR(MP_QSTR_argv), MP_ROM_PTR(&sys_argv_obj) },
	{ MP_ROM_QSTR(MP_QSTR_exit), MP_ROM_PTR(&sys_exit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&sys_read_obj) },
	{ MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&sys_write_obj) },
	{ MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&sys_open_obj) },
	{ MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&sys_close_obj) },
	{ MP_ROM_QSTR(MP_QSTR_lseek), MP_ROM_PTR(&sys_lseek_obj) },
	{ MP_ROM_QSTR(MP_QSTR_uname), MP_ROM_PTR(&sys_uname_obj) },
	{ MP_ROM_QSTR(MP_QSTR_getdents), MP_ROM_PTR(&sys_getdents_obj) },
	{ MP_ROM_QSTR(MP_QSTR_rename), MP_ROM_PTR(&sys_rename_obj) },
	{ MP_ROM_QSTR(MP_QSTR_mkdir), MP_ROM_PTR(&sys_mkdir_obj) },
	{ MP_ROM_QSTR(MP_QSTR_rmdir), MP_ROM_PTR(&sys_rmdir_obj) },
	{ MP_ROM_QSTR(MP_QSTR_remove), MP_ROM_PTR(&sys_remove_obj) },

	{ MP_ROM_QSTR(MP_QSTR_surface_create), MP_ROM_PTR(&sys_surface_create_obj) },
	{ MP_ROM_QSTR(MP_QSTR_surface_update), MP_ROM_PTR(&sys_surface_update_obj) },
	{ MP_ROM_QSTR(MP_QSTR_c_buffer_uint32_set), MP_ROM_PTR(&sys_c_buffer_uint32_set_obj) },

	{ MP_ROM_QSTR(MP_QSTR_sbrk), MP_ROM_PTR(&sys_sbrk_obj) },
	{ MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&sys_time_obj) },
	{ MP_ROM_QSTR(MP_QSTR_clock), MP_ROM_PTR(&sys_clock_obj) },
	{ MP_ROM_QSTR(MP_QSTR_random_u32), MP_ROM_PTR(&sys_random_u32_obj) },

	{ MP_ROM_QSTR(MP_QSTR_exec), MP_ROM_PTR(&sys_exec_obj) },
	{ MP_ROM_QSTR(MP_QSTR_kill), MP_ROM_PTR(&sys_kill_obj) },
	{ MP_ROM_QSTR(MP_QSTR_pstate), MP_ROM_PTR(&sys_pstate_obj) },
};

static MP_DEFINE_CONST_DICT(
	sys_module_globals, sys_module_globals_table
);

const mp_obj_module_t sys_module = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&sys_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_sys, sys_module);
