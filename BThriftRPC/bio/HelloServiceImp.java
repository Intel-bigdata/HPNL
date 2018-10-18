package main.bio;

import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocol;

public class HelloServiceImp implements HelloService.Iface {
    @Override
    public String hello(String s) throws TException {
        return s+"---hi";
    }


}
