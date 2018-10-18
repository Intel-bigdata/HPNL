package main.nio;


import org.apache.thrift.TProcessor;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.server.TNonblockingServer;
import org.apache.thrift.server.TServer;
import org.apache.thrift.transport.TFastFramedTransport;
import org.apache.thrift.transport.TNonblockingServerSocket;
import org.apache.thrift.transport.TTransportException;

public class Server {
    public static void main(String[] args) {
        TProcessor tProcessor=new HelloService.Processor<HelloService.Iface>(new HelloServiceImp());
        try {
            TNonblockingServerSocket serverSocket=new TNonblockingServerSocket(8877);
            TNonblockingServer.Args targs=new TNonblockingServer.Args(serverSocket);
            targs.processor(tProcessor);
            targs.transportFactory(new TFastFramedTransport.Factory());
//            targs.processorFactory(new TBinaryProtocol());
            TServer server=new TNonblockingServer(targs);
            System.out.println("server start ...");
            server.serve();
        } catch (TTransportException e) {
            e.printStackTrace();
        }

    }
}
