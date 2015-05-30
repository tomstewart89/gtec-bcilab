#ifndef SMARTINGAMP_H
#define	SMARTINGAMP_H

#ifndef _WIN32_WINNT            // Minimum platform is Windows 7
#define _WIN32_WINNT 0x0601
#endif

#include <iostream>
#include <stdexcept>
#include <queue>
#include <deque>
#include <list>
#include <string>
#include <algorithm>


#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>

#define OK_RESPONSE_SIZE 4
#define MAX_PACKAGE_SIZE 4096
#define MAX_PORT_SIZE 1000000
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

enum Command
{
	SELECT_CHANNELS,
	ON,
	OFF,
	TEST,
	NORMAL,
	REFON,
	REFOFF,
	IMPON,
	IMPOFF,
	FREQUENCY_250,
	FREQUENCY_500,
	NOISE
};

enum CommandStatus
{
    Success,
    Failure,
    TimeoutExpired
};

/**
 * \class SmartingAmp
 * \author mBrainTrain dev team (mBrainTrain)
 * \brief Communicates with a MBTSmarting device.
 */

class SmartingAmp : private boost::noncopyable
{

public:

	SmartingAmp();
	~SmartingAmp();

	bool connect(std::string& port_name);

	void disconnect();

	bool start();

	bool stop();

	/* NOTE: off doesn't wait for response */
	void off();

	CommandStatus send_command(Command cmd);

	CommandStatus command_status(unsigned char* response, int readed);

    void write(const unsigned char *data, const size_t& size);

    int read(unsigned char *data, size_t size);

	void acquire();

	void on_receive(const boost::system::error_code& ec, size_t bytes_transferred);

	float* get_sample();
	
	float* convert_data(std::vector<unsigned char> byte_array);

	void read_with_timeout(int size, size_t timeout);

	void read_complete(const boost::system::error_code& error, size_t bytes_transferred);

	void timeout_expired(const boost::system::error_code& error);

	float get_channel_value(unsigned char first, unsigned char second, unsigned char third);

	float get_gyro_value(unsigned char first, unsigned char second);

private:

	std::pair<unsigned char*, int> make_command(Command cmd);
   
	boost::asio::io_service m_io;
    
	boost::shared_ptr< boost::asio::serial_port > m_port;

	unsigned char m_receiveBuffer[MAX_PACKAGE_SIZE];
	
	unsigned char m_commandReceiveBuffer[MAX_PORT_SIZE];

	std::queue<float*> m_samplesBuffer;
	
	std::vector<unsigned char> m_byteArray;

	boost::mutex m_on_receive_lock;
	
	boost::mutex m_read_complete_lock;
	
	boost::mutex m_samples_lock;
	
	boost::mutex m_bytes_readed_lock;
	
	boost::mutex m_timer_expired_lock;

	int m_failedSamples;
	
	int m_receivedSamples;

	boost::shared_ptr< boost::thread > acquire_t;

	boost::shared_ptr< boost::asio::deadline_timer > m_timer;

	// read with timeout
	int m_bytes_readed;
};

#endif

