project(
    'iwgtk',
    'c',
    version: '0.8',
    license: 'GPL-3.0-or-later',
    meson_version: '>=0.60.0'
)

prefix = get_option('prefix')
sysconfdir = get_option('sysconfdir')
datadir = get_option('datadir')
mandir = get_option('mandir')
localedir = get_option('localedir')

add_project_arguments(
    '-D SYSCONFDIR="@0@"'.format(prefix / sysconfdir),
    '-D LOCALEDIR="@0@"'.format(prefix / localedir),
    '-D PACKAGE="@0@"'.format(meson.project_name()),
    '-D VERSION="@0@"'.format(meson.project_version()),
    language: 'c'
)

subdir('src')

dependencies = [
    dependency('gtk4', version: '>=4.6'),
    dependency('libqrencode')
]

executable(
    'iwgtk',
    sources: src_files,
    dependencies: dependencies,
    install: true
)

subdir('po')

scdoc = find_program('scdoc', native: true)

foreach i: [1,5]
    name = 'iwgtk.@0@'.format(i)
    src = 'misc/@0@.scd'.format(name)
    dir = mandir / 'man@0@'.format(i)

    custom_target(
        input: src,
        output: name,
        command: scdoc,
        feed: true,
        capture: true,
        install: true,
        install_dir: dir
    )
endforeach

install_data(
    'misc/iwgtk.conf',
    install_dir: sysconfdir
)

install_data(
    'misc/iwgtk-indicator.desktop',
    install_dir: sysconfdir / 'xdg/autostart'
)

install_data(
    'misc/iwgtk.service',
    install_dir: prefix / 'lib/systemd/user'
)

install_data(
    'misc/iwgtk.desktop',
    install_dir: datadir / 'applications'
)

install_data(
    'misc/iwgtk.svg',
    install_dir: datadir / 'icons/hicolor/scalable/apps'
)
