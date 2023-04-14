add_executable(bluetoothTest.elf
main.c
)

target_link_libraries(bluetoothTest.elf ${330_LIBS} lasertag )
set_target_properties(bluetoothTest.elf PROPERTIES LINKER_LANGUAGE CXX)
