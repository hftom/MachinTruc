TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
	core \
	app

app.depends = core

TRANSLATIONS = \
	machintruc_fr.ts \
	machintruc_es.ts
