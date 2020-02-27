project('vmread', 'c', 'cpp',
        default_options : ['c_std=c99', 'cpp_std=c++17'])


compile_args = ['-D_POSIX_C_SOURCE=200809L', '-pedantic', '-DMVERBOSE=4', '-DREAD_CHECK']
compile_args_external = ['-DLMODE=MODE_EXTERNAL', '-fsanitize=address']
compile_args_internal = ['-DLMODE=MODE_QEMU_INJECT', '-DMVERBOSE=4']
link_args_external = ['-lasan']
link_args_internal = []
link_args = []
cpp_compile_args = []
c_compile_args = []

try_args = ['-Weverything', '-Wno-padded', '-Wno-sign-conversion', '-Wno-padded', '-Wno-documentation-unknown-command', '-Wno-missing-variable-declarations', '-Wno-c++98-compat', '-Wno-c++98-compat-pedantic', '-Wno-weak-vtables', '-Wno-old-style-cast']
try_args_c = ['-Wno-missing-prototypes']
try_args_cpp = ['-Weverything -Wall -Wno-zero-as-null-pointer-constant -Wno-sign-conversion', '-Wno-covered-switch-default', '-Wno-c++98-compat', '-Wno-padded', '-Wno-c++98-compat-pedantic', '-Wno-weak-vtables', '-Wno-documentation-unknown-command', '-Wno-old-style-cast', '-Wno-float-equal', '-Wno-unused-command-line-argument -Wno-reserved-id-macro -Wno-cast-align -Wno-exit-time-destructors -Wno-global-constructors', '-fpermissive', '-Wno-padded', '-Wno-c++98-compat', '-Wno-padded', '-Wno-c++98-compat-pedantic', '-Wno-weak-vtables', '-Wno-documentation-unknown-command', '-Wno-old-style-cast']

cxx = meson.get_compiler('cpp')
cc = meson.get_compiler('c')

foreach arg : try_args
  if cc.has_argument(arg)
    compile_args += arg
  elif cxx.has_argument(arg)
    cpp_compile_args += arg
  endif
endforeach

foreach arg : try_args_cpp
  if cc.has_argument(arg)
    cpp_compile_args += arg
  endif
endforeach

foreach arg : try_args_c
  if cc.has_argument(arg)
    c_compile_args += arg
  endif
endforeach

dl = meson.get_compiler('c').find_library('dl', required : false)
thread = meson.get_compiler('c').find_library('pthread', required : false)

base_files = ['mem.c', 'wintools.c', 'pmparser.c']
hlapi_files = ['hlapi/windll.cpp', 'hlapi/winprocess.cpp', 'hlapi/winprocesslist.cpp']
modular_files = ['m0dular/utils/threading.cpp', 'm0dular/utils/semaphores.cpp', 'm0dular/utils/atomic_lock.cpp', 'm0dular/utils/mutex.cpp']

example = executable(
	'example',
	files(base_files + ['example.cpp', 'vmmem.c'] + hlapi_files),
  c_args : c_compile_args + compile_args + compile_args_external,
  cpp_args : cpp_compile_args + compile_args + compile_args_external,
  link_args : compile_args + compile_args_external + link_args + link_args_external
)

example_kmod = executable(
	'kmod_example',
	files(base_files + ['example.cpp', 'intmem.c'] + hlapi_files),
  c_args : c_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP'],
  cpp_args : cpp_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP'],
  link_args : compile_args + compile_args_external + link_args + link_args_external
)

example_kmod_src = executable(
	'kmod_test',
	files(base_files + ['src/utils/Handles.cpp', 'src/utils/Memutils.cpp', 'src/utils/Logger.cpp', 'src/test.cpp', 'intmem.c'] + hlapi_files + modular_files),
  c_args : c_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
  cpp_args : cpp_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
  link_args : compile_args + compile_args_external + link_args + link_args_external
)

example_kmod_src = executable(
	'kmod_src',
	files(base_files + ['src/utils/Handles.cpp', 'src/utils/Memutils.cpp', 'src/utils/Logger.cpp', 'src/Main.cpp', 'intmem.c'] + hlapi_files + modular_files),
  c_args : c_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
  cpp_args : cpp_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
  link_args : compile_args + compile_args_external + link_args + link_args_external
)

# example = executable(
# 	'ttt',
# 	files(base_files + ['eft/math.cpp', 'eft/eftstructs.cpp', 'eft/main.cpp', 'vmmem.c'] + hlapi_files),
#   c_args : c_compile_args + compile_args + compile_args_external,
#   cpp_args : cpp_compile_args + compile_args + compile_args_external,
#   link_args : compile_args + compile_args_external + link_args + link_args_external
# )

example_kmod_src = executable(
	'ttt',
	files(base_files + ['eft/math.cpp', 'eft/eftstructs.cpp', 'eft/main.cpp', 'intmem.c'] + hlapi_files + modular_files),
  c_args : c_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
  cpp_args : cpp_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
  link_args : compile_args + compile_args_external + link_args + link_args_external
)

# example_kmod_src = executable(
# 	'internal_src',
# 	files(base_files + ['internal/eftstructs.cpp', 'internal/main.cpp', 'intmem.c'] + hlapi_files + modular_files),
#   c_args : c_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
#   cpp_args : cpp_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4', '-DMEMUTILS_CUSTOM_RW', '-DPATTERN_SCAN_PAGE_SCAN'],
#   link_args : compile_args + compile_args_external + link_args + link_args_external
# )

# example_kmod_src = executable(
# 	'kmod_src',
# 	files(base_files + ['src/Main.cpp', 'intmem.c'] + hlapi_files + modular_files + ['src/utils/Handles.cpp', 'src/utils/Memutils.cpp', 'src/utils/Logger.cpp']),
#   c_args : c_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4'],
#   cpp_args : cpp_compile_args + compile_args + compile_args_external + ['-DKMOD_MEMMAP', '-DNUM_THREADS=4'],
#   link_args : compile_args + compile_args_external + link_args + link_args_external
# )

example_lib = shared_library(
	'example',
	files(base_files + ['example.cpp', 'intmem.c'] + hlapi_files),
  c_args : compile_args + compile_args_internal,
  cpp_args : compile_args + compile_args_internal,
  link_args : compile_args + compile_args_internal + link_args + link_args_internal,
  dependencies: [dl]
)
