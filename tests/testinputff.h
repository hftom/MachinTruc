#ifndef TESTINPUTFF_H
#define TESTINPUTFF_H

#include "AutoTest.h"



class TestInputFF : public QObject
{
    Q_OBJECT

private slots:
    void probeReturnsTrue();
	void probeReturnsFalse();
    void test2();
};

DECLARE_TEST(TestInputFF)

#endif // TESTINPUTFF_H
