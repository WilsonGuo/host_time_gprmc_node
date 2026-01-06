#include <ros/ros.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

/* ========== 串口工具 ========== */

int open_serial(const std::string& port, int baudrate)
{
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        perror("open serial");
        return -1;
    }

    termios tty{};
    tcgetattr(fd, &tty);

    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;

    tcsetattr(fd, TCSANOW, &tty);
    return fd;
}

/* ========== NMEA 校验 ========== */

uint8_t nmea_checksum(const std::string& s)
{
    uint8_t cs = 0;
    for (char c : s)
        cs ^= static_cast<uint8_t>(c);
    return cs;
}

/* ========== GPRMC 生成 ========== */

std::string generate_gprmc()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto tt  = system_clock::to_time_t(now);
    std::tm utc{};
    gmtime_r(&tt, &utc);

    char buf[128];

    snprintf(buf, sizeof(buf),
        "GPRMC,%02d%02d%02d.00,A,0000.00,N,00000.00,E,0.00,0.00,%02d%02d%02d,,,A",
        utc.tm_hour,
        utc.tm_min,
        utc.tm_sec,
        utc.tm_mday,
        utc.tm_mon + 1,
        (utc.tm_year + 1900) % 100
    );

    uint8_t cs = nmea_checksum(buf);
    char out[160];
    snprintf(out, sizeof(out), "$%s*%02X\r\n", buf, cs);
    return std::string(out);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "host_time_gprmc_node");
    ros::NodeHandle nh("~");

    std::string port;
    int baud;

    nh.param<std::string>("port", port, "/dev/ttyACM0");
    nh.param<int>("baud", baud, B9600);
    ROS_WARN("Host GPRMC time sync  will opem on %s", port.c_str());

    int fd = open_serial(port, baud);
    if (fd < 0)
        return -1;

    ROS_INFO("Host GPRMC time sync started on %s", port.c_str());

    ros::Rate rate(1.0);

    while (ros::ok())
    {
        std::string gprmc = generate_gprmc();
        write(fd, gprmc.c_str(), gprmc.size());

        ROS_INFO_STREAM("TX (UTC Time): " << gprmc.substr(0, gprmc.size() - 2));

        ros::spinOnce();
        rate.sleep();
    }

    close(fd);
    return 0;
}
