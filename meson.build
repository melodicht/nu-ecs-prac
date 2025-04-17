project('sdl_ecsprac', 'cpp',
        default_options: 'default_library=static')

sdl2_dep = dependency('sdl2')
imgui_dep = dependency('imgui')
glm_dep = dependency('glm')

incdir = include_directories('include')

executable('main', 'src/sdl_ecsprac.cpp', 'src/glad.cpp',
           dependencies: [imgui_dep, sdl2_dep, glm_dep],
           include_directories : incdir)


