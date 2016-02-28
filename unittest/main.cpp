#include "cbortest.h"

int main(int argc, char *argv[])
{
    CBORTest cborTest;
    QTest::qExec(&cborTest, argc, argv);

    return 0;
}
