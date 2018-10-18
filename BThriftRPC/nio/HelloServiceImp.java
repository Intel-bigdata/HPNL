    package main.nio;

    import org.apache.thrift.TException;

    public class HelloServiceImp implements HelloService.Iface {
        @Override
        public String hello(String s) throws TException {
            return s+"---hi";
        }


    }
