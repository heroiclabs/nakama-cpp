#include "nakama-cpp/nakama.h"
#include <grpc++/create_channel.h>
#include "api/github.com/heroiclabs/nakama/apigrpc/apigrpc.grpc.pb.h"

namespace nakama {

void do_test()
{
	auto channel = grpc::CreateChannel("127.0.0.1:7349", grpc::InsecureChannelCredentials());

	auto stub = nakama::api::Nakama::NewStub(channel);
	grpc::ClientContext context;

	nakama::api::AuthenticateDeviceRequest req;
	nakama::api::Session session;

	req.mutable_account()->set_id("mytestdevice0000");
	req.mutable_create()->set_value(true);

	//stub->AsyncAuthenticateDevice();
	auto status = stub->AuthenticateDevice(&context, req, &session);

	if (status.ok())
	{
		std::cout << "OK: token=" << session.token() << std::endl;
	}
	else
	{
		std::cout << "Failed: error_code=" << status.error_code() <<
			", details=" << status.error_details() <<
			", message=" << status.error_message() << std::endl;
	}
}

} // namespace nakama
