package com.intel.hpnl.buffer;

import com.intel.hpnl.core.FixedSizeBufferAllocator;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;
import org.junit.Test;
import sun.misc.Unsafe;
import sun.nio.ch.DirectBuffer;

import java.lang.reflect.Field;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;

public class PooledBufferTest {

    static volatile ByteBuf[] globalBuf = new ByteBuf[5];

    static{
        System.load("/apps/HPNL/java/hpnl/src/test/java/com/intel/hpnl/buffer/test.so");
    }

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

//    @Test
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

    @Test
    public void testJni()throws Exception{
        PooledByteBufAllocator allocator = new PooledByteBufAllocator();
        int size = 10*1024*1024;
        ByteBuf buf = allocator.directBuffer(size);
        int count = 0;
        String msg = "bcdefghijk";
        while(count < size){
            buf.writeBytes(msg.getBytes());
            count += msg.length();
        }
        long addr = buf.memoryAddress();
        System.out.println("addr: "+addr);
        Utils.verify(addr);
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

    @Test
    public void testRpcNettyCache()throws Exception{
        int directArena = Runtime.getRuntime().availableProcessors()*2; // specified or core size*2
        int tinyCacheSize = 512;// specified or 512
        int smallCacheSize = 256;// specified or 256
        int normalCacheSize = 64;// specified or 64
        PooledByteBufAllocator allocator = new PooledByteBufAllocator(true, 0, directArena,
        8192, 11, tinyCacheSize, smallCacheSize, normalCacheSize, true);
        ByteBuf buffer = allocator.directBuffer();
        System.out.println(buffer.capacity());
    }

    @Test
    public void testBufferCopy()throws Exception{
        ByteBuffer src = ByteBuffer.allocateDirect(16);
        ByteBuffer buffer2 = ByteBuffer.allocateDirect(16);
        Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
        unsafeField.setAccessible(true);
        Unsafe unsafe = (Unsafe)unsafeField.get(null);
        src.putLong(2).putLong(3);
        src.position(8);
        buffer2.putLong(4);
        int length = 8;
        long memoryAddress = ((DirectBuffer)buffer2).address();
        if(unsafe != null && src.isDirect() && (memoryAddress > 0)) {
            if(length < 0){
                throw new IllegalArgumentException("length should be no less than 0, "+length);
            }
            if(src.remaining() < length || buffer2.remaining() < length){
                throw new IllegalArgumentException("both source buffer and dest buffer's remaining should no less than length, "+length);
            }
            unsafe.copyMemory(((DirectBuffer)src).address()+src.position(),
                    memoryAddress+buffer2.position(), length);
            buffer2.position(8);
            src.position(src.position()+length);
            System.out.println(buffer2.getLong());
        }
    }

    @Test
    public void testLazySet()throws Exception{
        AtomicInteger value = new AtomicInteger(0);
        Runnable task = () -> {
            for (int i = 0; i < 1000; i++) {
                value.lazySet(i+ 1);
            }
            while (!Thread.currentThread().isInterrupted()){
                try {
                    Thread.sleep(1);
                }catch (Exception e){
                    e.printStackTrace();
                }
            }
        };

        Runnable task2 = () -> {
            while(true) {
                int i = value.get();
                System.out.println(Thread.currentThread().getName() + ": " + i);
                try {
                    Thread.sleep(2);
                }catch (Exception e){
                    e.printStackTrace();
                }
                if(i == 1000){
                    System.out.println("done");
                    break;
                }
            }
        };
        Thread t1 = new Thread(task, "t1");
        Thread t2 = new Thread(task2, "t2");
        t1.start();
        t2.start();
        t1.join();
        t2.join();

        System.out.println(value.intValue());

    }

    @Test
    public void testMetaSize()throws Exception{
        System.out.println(FixedSizeBufferAllocator.BUFFER_METADATA_SIZE);
    }
}
