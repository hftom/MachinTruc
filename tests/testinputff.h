#ifndef TESTINPUTFF_H
#define TESTINPUTFF_H

#include "AutoTest.h"



class TestInputFF : public QObject
{
    Q_OBJECT

private slots:
    void probeReturnsTrue();
	void probeReturnsFalse();
	void streamDurationCorrectlyDetected();
	void allFramesDecoded();
	void seekBackOneFrameFromEnd();
	void seekStart();
	void resampleDoubleFrameRate();
	void resampleTripleFrameRate();
	void resampleHalfFrameRate();
	void memLeakTest();
};

DECLARE_TEST(TestInputFF)

#endif // TESTINPUTFF_H
