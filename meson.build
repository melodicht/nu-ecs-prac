project('untitled_engine', 'cpp',
        default_options: ['default_library=static','cpp_std=c++20'])

slangc = find_program('slangc')

sdl3_dep = dependency('sdl3')
imgui_dep = dependency('imgui')
glm_dep = dependency('glm')
vulkan_dep = dependency('vulkan', method: 'auto')

incdir = include_directories('include')

shaders = ['shaders/shader.slang']
shader_targets = []
foreach s : shaders
    shader_targets += custom_target(
        'shader @0@'.format(s.substring(8)),
        command : [slangc, '@INPUT@', '-o', 'shaders/@OUTPUT@', '-target', 'spirv', '-fvk-use-entrypoint-name', '-emit-spirv-directly', '-fvk-use-gl-layout', '-matrix-layout-column-major'],
        input : s,
        output : '@BASENAME@.spv'.format(s),
        build_always_stale : true,
        build_by_default : true
    )
endforeach


executable('main', 'src/main.cpp', 'src/VkBootstrap.cpp',  shader_targets,
           dependencies: [imgui_dep, sdl3_dep, glm_dep, vulkan_dep],
           include_directories : incdir)


