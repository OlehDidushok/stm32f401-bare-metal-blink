
Import("env")

platform = env.PioPlatform()
openocd_dir = platform.get_package_dir("tool-openocd")

env.Replace(
    UPLOADCMD=(
        f'"{openocd_dir}/bin/openocd" '
        f'-s "{openocd_dir}/openocd/scripts" '
        '-d3 '
        '-f interface/stlink.cfg '
        '-c "transport select swd" '
        '-c "adapter speed 400" '
        '-f target/stm32f4x.cfg '
        '-c "program {$BUILD_DIR/${PROGNAME}.elf} verify reset; shutdown;"'
    )
)