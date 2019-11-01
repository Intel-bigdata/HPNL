package com.intel.hpnl.buffer;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import org.junit.Test;
import sun.misc.Unsafe;

import java.lang.reflect.Field;

public class PooledBufferTest {

    static volatile ByteBuf[] globalBuf = new ByteBuf[5];

//    @Test
    public void testAllocation()throws Exception{
        PooledByteBufAllocator allocator = new PooledByteBufAllocator();
        Th t1 = new Th(allocator);
        t1.start();
        Thread.sleep(200);

        Th t2 = new Th(allocator);
        t2.start();
        t2.complete();
        t2.join();

        t1.go();
        t1.join();
    }

    @Test
    public void testLargeAllocator()throws Exception{
        PooledByteBufAllocator allocator = new PooledByteBufAllocator();
        int size = 10*1024*1024;
        ByteBuf buf = allocator.directBuffer(size);
        int count = 0;
        String msg = "bcdefghijk";
        int hash = 0;
        while(count < size){
            buf.writeBytes(msg.getBytes());
            count += msg.length();
            hash += msg.hashCode();
        }
        System.out.println("hash1: "+hash);
//        buf.writeBytes(msg.getBytes());
        if(buf.hasMemoryAddress()) {
            long addr = buf.memoryAddress();
            System.out.println(addr);
            Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
            unsafeField.setAccessible(true);
            Unsafe unsafe = (Unsafe)unsafeField.get(null);
            ByteBuf buf2 = allocator.directBuffer(size);
            unsafe.copyMemory(addr, buf2.memoryAddress(), size);
//            buf2.writerIndex(size);
            count = 0;
            int hash2 = 0;
            byte[] bytes = new byte[msg.length()];
            while(count < size) {
                buf2.getBytes(count, bytes);
                count += msg.length();
                hash2 += new String(bytes).hashCode();
            }
            System.out.println("hash2: "+hash2);
        }

    }

    class Th extends Thread{
        volatile int status = 0;
        PooledByteBufAllocator allocator;
        public Th(PooledByteBufAllocator allocator){
            this.allocator = allocator;
        }

        @Override
        public void run() {
//            ByteBuf buf = null;
            for(int i=0; i<5; i++) {
                if(globalBuf[i] != null){
                    globalBuf[i].release();
                }
            }
            for(int i=0; i<5; i++) {
                globalBuf[i] = allocator.directBuffer();
//                buf.release();
            }
            while(status == 0){
                try {
                    Thread.sleep(20);
                }catch (InterruptedException e){}
            }
            if(status == 1){
                return;
            }
            for(int i=0; i<5; i++) {
                globalBuf[i] = allocator.directBuffer();
//                buf.release();
            }
        }

        public void complete() {
            status = 1;
        }

        public void go() {
            status = 2;
        }
    }
}
