# SPDX-License-Identifier: GPL-2.0-or-later
if host_machine.system() == 'windows'

sources += [
  'util/argconfig.c',
  'util/base64.c',
  'util/crc32.c',
  'util/mem.c',
  'util/sighdl.c',
  'util/suffix.c',
  'util/types.c',
  'util/utils.c',
  'util/table.c'
]

else
sources += [
  'util/argconfig.c',
  'util/base64.c',
  'util/crc32.c',
  'util/mem.c',
  'util/sighdl.c',
  'util/suffix.c',
  'util/types.c',
  'util/utils.c',
  'util/table.c'
]
endif

if json_c_dep.found()
  sources += [
    'util/json.c',
  ]
endif
