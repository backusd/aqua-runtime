
add_executable(aqua-minimal-app
	main.cpp
)

target_link_libraries(aqua-minimal-app PRIVATE aqua-runtime)

target_compile_features(aqua-minimal-app PRIVATE cxx_std_23)

