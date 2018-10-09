#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <map>


using namespace std;

#define CYLINK_STX_FLAG (0xfe)
#define CYLINK_TPY_DATA (0)
#define CYLINK_TPY_CONTROL (1)



//router 
map<uint32_t, boost::asio::ip::udp::endpoint> router_;

//注册本地路由信息
void register_router(uint32_t uid, boost::asio::ip::udp::endpoint ep)
{
	if (router_.count(uid) == 0) {
		
		router_.insert(pair<uint32_t, boost::asio::ip::udp::endpoint>(uid, ep));
		cout << "路由信息不存在，增加路由信息" << endl;
	}
}

//转发
void transpond(boost::asio::ip::udp::socket &socket_,char *buffer,int size,uint32_t uid)
{
	map<uint32_t, boost::asio::ip::udp::endpoint>::iterator iter_;

	cout << "查找路由表" << endl;
	iter_ = router_.find(uid);
	

	if (iter_ != router_.end()) {
		cout << "转发目标：" << iter_->second.address() << endl;

		socket_.send_to(boost::asio::buffer(buffer, size), iter_->second);
	}
	else {
		cout << "转发目标" << uid << "不存在" << endl;
	}
}

int main(int argc,char *argv[]) 
{


	cout << "CYLINK 服务端" << endl;


	boost::asio::io_service io_service_;
	boost::asio::ip::udp::socket socket_(io_service_,boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),18888));


	while (true) {

		boost::array<char, 65535> recv_buf_;

		boost::asio::ip::udp::endpoint remote_endpoint_;

		boost::system::error_code error_code_;

		socket_.receive_from(boost::asio::buffer(recv_buf_), remote_endpoint_, 0, error_code_);



		int length_ = recv_buf_[1] << 8 | recv_buf_[2];
		int seq_ = recv_buf_[3];
		int type_ = recv_buf_[4];
		int message_ = recv_buf_[5];
		uint32_t suid_ = recv_buf_[6] << 24 | recv_buf_[7] << 16 | recv_buf_[8] << 8 | recv_buf_[9];
		uint32_t duid_ = recv_buf_[10] << 24 | recv_buf_[11] << 16 | recv_buf_[12] << 8 | recv_buf_[13];


		if (recv_buf_[0] == CYLINK_STX_FLAG) {
			cout << "获取到cylinkv1.0数据包 ["<< length_ <<"]" << endl;
			switch (type_)
			{
			case CYLINK_TPY_DATA:
				cout << "负载类型：透传帧(" << suid_ << "--->" << duid_ << ")" << endl;


				register_router(suid_, remote_endpoint_);//注册本地路由表

				transpond(socket_,recv_buf_.data(), length_ + 8, duid_);

				break;
			case CYLINK_TPY_CONTROL:
				cout << "负载类型：控制帧("<< suid_ <<")" << endl;

				register_router(suid_, remote_endpoint_);//注册本地路由表


				break;
			default:
				break;
			} 

		}
		else {
			cout << "不符合cylink1.0数据协议" << endl;
		}


	}

	system("pause");

}


