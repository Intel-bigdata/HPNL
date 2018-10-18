package main.bio;

import org.apache.thrift.TException;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

public class Client {
    public static void main(String[] args) {
        TTransport transport=new TSocket("localhost",7788);
        try {
            transport.open();
            TProtocol protocol = new  TBinaryProtocol(transport);
            HelloService.Client hi=new HelloService.Client(protocol);
           String s= hi.hello("12");
            System.out.println(s);

        } catch (TException e) {
            e.printStackTrace();
        }

    }
}
