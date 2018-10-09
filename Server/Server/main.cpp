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

//ע�᱾��·����Ϣ
void register_router(uint32_t uid, boost::asio::ip::udp::endpoint ep)
{
	if (router_.count(uid) == 0) {
		
		router_.insert(pair<uint32_t, boost::asio::ip::udp::endpoint>(uid, ep));
		cout << "·����Ϣ�����ڣ�����·����Ϣ" << endl;
	}
}

//ת��
void transpond(boost::asio::ip::udp::socket &socket_,char *buffer,int size,uint32_t uid)
{
	map<uint32_t, boost::asio::ip::udp::endpoint>::iterator iter_;

	cout << "����·�ɱ�" << endl;
	iter_ = router_.find(uid);
	

	if (iter_ != router_.end()) {
		cout << "ת��Ŀ�꣺" << iter_->second.address() << endl;

		socket_.send_to(boost::asio::buffer(buffer, size), iter_->second);
	}
	else {
		cout << "ת��Ŀ��" << uid << "������" << endl;
	}
}

int main(int argc,char *argv[]) 
{


	cout << "CYLINK �����" << endl;


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
			cout << "��ȡ��cylinkv1.0���ݰ� ["<< length_ <<"]" << endl;
			switch (type_)
			{
			case CYLINK_TPY_DATA:
				cout << "�������ͣ�͸��֡(" << suid_ << "--->" << duid_ << ")" << endl;


				register_router(suid_, remote_endpoint_);//ע�᱾��·�ɱ�

				transpond(socket_,recv_buf_.data(), length_ + 8, duid_);

				break;
			case CYLINK_TPY_CONTROL:
				cout << "�������ͣ�����֡("<< suid_ <<")" << endl;

				register_router(suid_, remote_endpoint_);//ע�᱾��·�ɱ�


				break;
			default:
				break;
			} 

		}
		else {
			cout << "������cylink1.0����Э��" << endl;
		}


	}

	system("pause");

}


