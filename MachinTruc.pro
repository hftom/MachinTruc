TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
	core \
	app

app.depends = core
