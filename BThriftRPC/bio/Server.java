package main.bio;

import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TSimpleServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TTransportException;

public class Server {
    public static void main(String[] args) {
        HelloServiceImp helloServiceImp = new HelloServiceImp();
        HelloService.Processor processor = new HelloService.Processor(helloServiceImp);
        try {
            TServerSocket tServerSocket = new TServerSocket(7788);
            TServer server=new TSimpleServer(new TServer.Args(tServerSocket).processor(processor));
            System.out.println("server start");
            server.serve();
        } catch (TTransportException e) {
            e.printStackTrace();
        }

    }
}
