#include "input/input_ff.h"

#include "testinputff.h"



void TestInputFF::probeReturnsTrue()
{
	InputFF *in = new InputFF();
	Profile prof;
	bool b = in->probe( "/home/cris/Videos/h264/vitre.mkv", &prof );
	delete in;
    QVERIFY( b == true );
}



void TestInputFF::probeReturnsFalse()
{
	InputFF *in = new InputFF();
	Profile prof;
	bool b = in->probe( "testinputff.h", &prof );
	delete in;
    QVERIFY( b == false );
}



void TestInputFF::test2()
{
    QVERIFY(1 == 0);
}
