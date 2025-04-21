project('sdl_ecsprac', 'cpp',
        default_options: 'default_library=static')

sdl3_dep = dependency('sdl3')
imgui_dep = dependency('imgui')
glm_dep = dependency('glm')

incdir = include_directories('include')

executable('main', 'src/main.cpp', 'src/glad.cpp',
           dependencies: [imgui_dep, sdl3_dep, glm_dep],
           include_directories : incdir)


