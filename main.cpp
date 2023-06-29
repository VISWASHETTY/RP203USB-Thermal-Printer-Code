#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QSerialPort>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Open the serial port for communication with the printer
    QSerialPort serialPort;
    serialPort.setPortName("COM3");  // Replace with the appropriate port name
    serialPort.setBaudRate(QSerialPort::Baud9600);  // Set the baud rate
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort.open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open serial port:" << serialPort.errorString();
        return -1;
    }

    // Prepare the text and image for printing
    QString textToPrint = "HELLO GREENMED";
    QImage imageToPrint("C:/images.png");  // Use forward slashes for the file path

    // Convert the image to a monochrome bitmap
    QImage monoImage = imageToPrint.convertToFormat(QImage::Format_Mono);

    // Calculate the number of dots per line and bytes per line
    int dotsPerLine = 384;
    int bytesPerLine = dotsPerLine / 8;

    // Create a QPainter for rendering text and image onto the bitmap
    QImage bitmap(dotsPerLine, 1, QImage::Format_Mono);
    QPainter painter(&bitmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(bitmap.rect(), Qt::white);  // Fill the bitmap with white background

    // Render the text onto the bitmap
    QFont font("Arial", 8);  // Adjust the font size based on the specifications
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(bitmap.rect(), Qt::AlignCenter, textToPrint);

    // Render the image onto the bitmap
    painter.drawImage(QPoint(0, 0), monoImage);

    // Convert the bitmap data into bytes
    QByteArray bytes;
    for (int y = 0; y < bitmap.height(); ++y) {
        const uchar* scanLine = bitmap.scanLine(y);
        for (int x = 0; x < bytesPerLine; ++x) {
            uchar byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int dotIndex = x * 8 + bit;
                if (dotIndex < dotsPerLine && scanLine[dotIndex] == 0)
                    byte |= (1 << bit);
            }
            bytes.append(byte);
        }
    }

    // Send the bytes to the printer for printing
    QByteArray command;
    command.append("\x1B\x40");  // Initialize printer
    command.append("\x1B\x2A\x21\x00\x01");  // Set print mode: double width and height
    command.append("\x1B\x2A\x21\x00\x00");  // Cancel print mode: double width and height
    command.append("\x1B\x61\x01");  // Select center justification
    command.append(bytes);  // Append the bitmap data
    command.append("\n");  // Print and line feed

    serialPort.write(command);
    serialPort.waitForBytesWritten();

    // Close the serial port
    serialPort.close();

    return 0;
}
