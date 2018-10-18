package main.nio;


import org.apache.thrift.TException;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TFramedTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

public class Client {
    public static void main(String[] args) {
        TTransport transport=new TFramedTransport(new TSocket("localhost",8877));

        TProtocol protocol=new TBinaryProtocol(transport);
        HelloService.Client hi=new HelloService.Client(protocol);
        try {
            transport.open();
            String s=hi.hello("zhang");
            System.out.println(s);
        } catch (TTransportException e) {
            e.printStackTrace();
        } catch (TException e) {
            e.printStackTrace();
        }


    }
}
