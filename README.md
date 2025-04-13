## Design Notes

- The reason why `u64` is used for EntityID is to avoid narrowing. We use
  `std::vector::size` for getting unique EntityIDs, which outputs `u64`.

# Setting up Meson build
First make sure you have Meson installed. Most of the need-to-know information for this setup can be extrapolated from reading [this page](https://mesonbuild.com/Quick-guide.html)
Clone the project and setup your Meson /build/ directory, as instructed from the link above.
Once you've setup the build directory, add a folder called `subprojects` to the root project folder, run the following command in the root of the project folder:
`meson wrap install imgui`
This will install the imgui, then once finished you can compile the project using
`meson compile -C <build-directory-name>`
and you can run the project by running:
`./<build-directory-name>/main`
