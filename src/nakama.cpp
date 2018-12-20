#include "nakama-cpp/nakama.h"
#include "nakama-cpp/strutil.h"
#include <grpc++/create_channel.h>
#include "api/github.com/heroiclabs/nakama/apigrpc/apigrpc.grpc.pb.h"

namespace nakama {

void do_test()
{
    auto channel = grpc::CreateChannel("127.0.0.1:7349", grpc::InsecureChannelCredentials());

    auto stub = nakama::api::Nakama::NewStub(channel);
    grpc::ClientContext context;

    context.AddMetadata("authorization", "Basic " + base64_encode("defaultkey:"));

    nakama::api::AuthenticateDeviceRequest req;
    nakama::api::Session session;

    req.mutable_account()->set_id("mytestdevice0000");
    req.mutable_create()->set_value(true);

    auto status = stub->AuthenticateDevice(&context, req, &session);

    if (status.ok())
    {
        std::cout << "OK: token=" << session.token() << std::endl;

        grpc::ClientContext context;
        context.AddMetadata("authorization", "Bearer " + session.token());

        grpc::CompletionQueue cq;
        std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::nakama::api::Account>> rpc;
        ::nakama::api::Account reply;

        rpc = stub->AsyncGetAccount(&context, google::protobuf::Empty(), &cq);

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the integer 1.
        rpc->Finish(&reply, &status, (void*)1);
        void* got_tag;
        bool ok = false;
        // Block until the next result is available in the completion queue "cq".
        // The return value of Next should always be checked. This return value
        // tells us whether there is any kind of event or the cq_ is shutting down.
        GPR_ASSERT(cq.Next(&got_tag, &ok));

        // Verify that the result from "cq" corresponds, by its tag, our previous
        // request.
        GPR_ASSERT(got_tag == (void*)1);
        // ... and that the request was completed successfully. Note that "ok"
        // corresponds solely to the request for updates introduced by Finish().
        GPR_ASSERT(ok);

        // Act upon the status of the actual RPC.
        if (status.ok()) {
            std::cout << "username: " << reply.user().username() << std::endl;
        }
        else {
            std::cout << "RPC failed" << std::endl;
            std::cout << "Failed: error_code=" << status.error_code() <<
                ", details=" << status.error_details() <<
                ", message=" << status.error_message() << std::endl;
        }
    }
    else
    {
        std::cout << "Failed: error_code=" << status.error_code() <<
            ", details=" << status.error_details() <<
            ", message=" << status.error_message() << std::endl;
    }
}

} // namespace nakama
