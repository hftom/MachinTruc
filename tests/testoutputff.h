#ifndef TESTOUTPUTFF_H
#define TESTOUTPUTFF_H

#include "AutoTest.h"



class TestOutputFF : public QObject
{
    Q_OBJECT

private slots:
	void encode();

};

DECLARE_TEST(TestOutputFF)

#endif // TESTOUTPUTFF_H
