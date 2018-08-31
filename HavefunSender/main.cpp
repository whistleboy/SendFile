#include "filesender.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileSender w;
    w.show();

    return a.exec();
}
