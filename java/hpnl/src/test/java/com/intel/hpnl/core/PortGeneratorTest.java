package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlConfig;
import org.apache.commons.io.FileUtils;
import org.junit.*;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.util.TreeSet;

@RunWith(PowerMockRunner.class)
public class PortGeneratorTest {
    private File portFile;
    private File tmpDir;

    @Before
    public void setup(){
        String tmp = System.getProperty("java.io.tmpdir");
        tmpDir = new File(tmp+"/hpnl/"+ HpnlConfig.getInstance().getAppId());
    }

    @Test(expected = IllegalStateException.class)
    public void testReadPortFileFailed()throws Exception{
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        portFile = Files.createTempFile("unit-test", ".port").toFile();
        try(FileWriter writer = new FileWriter(portFile)){
            writer.write("1-50");
            writer.write('\n');
            writer.write("51-100");
            writer.write('\n');
            writer.write("100-300");
            writer.write('\n');
        }
        Whitebox.invokeMethod(generator, "getPortsFromFile", portFile);
    }

    @Test
    public void testReadPortFromFileSucceed()throws Exception{
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        portFile = Files.createTempFile("unit-test", ".port").toFile();
        try(FileWriter writer = new FileWriter(portFile)){
            writer.write("1-50");
            writer.write('\n');
            writer.write("51-100");
            writer.write('\n');
            writer.write("101-300");
            writer.write('\n');
        }
        TreeSet<PortGenerator.Range> ranges = Whitebox.invokeMethod(generator, "getPortsFromFile", portFile);
        int i = 0;
        for(PortGenerator.Range r : ranges){
            switch (i){
                case 0: Assert.assertEquals("1-300", r.toString()); break;
                default: throw new Exception("should have only 1 entry");
            }
            i++;
        }
    }

    @Test
    public void testNewRange()throws Exception{
        new PortGenerator.Range(1 ,1);
        new PortGenerator.Range(2, 10);
    }

    @Test(expected = IllegalStateException.class)
    public void testRangeOutOfMaxValue()throws Exception{
        new PortGenerator.Range(1 ,65536);
    }

    @Test(expected = IllegalStateException.class)
    public void testRangeEndLowerThanStart()throws Exception{
        new PortGenerator.Range(100 ,99);
    }

    @Test
    public void testAddRange()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        base.add(new PortGenerator.Range(1, 30));
        base.add(new PortGenerator.Range(31, 50));
        base.add(new PortGenerator.Range(31, 50));
        Assert.assertEquals(2, base.size());
    }

    @Test(expected = IllegalStateException.class)
    public void testMergeRangeFailed()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        base.add(new PortGenerator.Range(1, 30));
        base.add(new PortGenerator.Range(31, 50));

        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(2, 30));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "mergeRange", base, ranges);
    }

    @Test(expected = IllegalStateException.class)
    public void testMergeRangeFailed2()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        base.add(new PortGenerator.Range(1, 30));
        base.add(new PortGenerator.Range(31, 50));
        base.add(new PortGenerator.Range(100, 150));

        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(50, 70));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "mergeRange", base, ranges);
    }

    @Test
    public void testMergeRange()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        base.add(new PortGenerator.Range(1, 30));
        base.add(new PortGenerator.Range(32, 50));
        base.add(new PortGenerator.Range(100, 150));
        base.add(new PortGenerator.Range(200, 250));
        base.add(new PortGenerator.Range(252, 300));
        base.add(new PortGenerator.Range(350, 400));

        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(51, 99));
        ranges.add(new PortGenerator.Range(301, 349));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "mergeRange", base, ranges);
        Assert.assertEquals(4, base.size());
        int i = 0;
        for(PortGenerator.Range r : base){
            switch (i){
                case 0: Assert.assertEquals("1-30", r.toString()); break;
                case 1: Assert.assertEquals("32-150", r.toString()); break;
                case 2: Assert.assertEquals("200-250", r.toString()); break;
                case 3: Assert.assertEquals("252-400", r.toString()); break;
            }
            i++;
        }
    }

    @Test
    public void testMergeRange2()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();

        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(51, 99));
        ranges.add(new PortGenerator.Range(301, 349));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "mergeRange", base, ranges);
        Assert.assertEquals(2, base.size());
    }

    @Test
    public void testMergeRange3()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        base.add(new PortGenerator.Range(32, 50));
        base.add(new PortGenerator.Range(100, 150));
        base.add(new PortGenerator.Range(200, 250));
        base.add(new PortGenerator.Range(252, 300));
        base.add(new PortGenerator.Range(350, 400));

        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 30));
        ranges.add(new PortGenerator.Range(51, 99));
        ranges.add(new PortGenerator.Range(301, 349));
        ranges.add(new PortGenerator.Range(600, 700));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "mergeRange", base, ranges);
        Assert.assertEquals(5, base.size());
        int i = 0;
        for(PortGenerator.Range r : base){
            switch (i){
                case 0: Assert.assertEquals("1-30", r.toString()); break;
                case 1: Assert.assertEquals("32-150", r.toString()); break;
                case 2: Assert.assertEquals("200-250", r.toString()); break;
                case 3: Assert.assertEquals("252-400", r.toString()); break;
                case 4: Assert.assertEquals("600-700", r.toString()); break;
            }
            i++;
        }
    }

    @Test
    public void testRevert()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 30));
        ranges.add(new PortGenerator.Range(51, 99));
        ranges.add(new PortGenerator.Range(301, 349));
        ranges.add(new PortGenerator.Range(600, 700));
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "revert", base, ranges);
        Assert.assertEquals(4, base.size());

        int i = 0;
        for(PortGenerator.Range r : base){
            switch (i){
                case 0: Assert.assertEquals("31-50", r.toString()); break;
                case 1: Assert.assertEquals("100-300", r.toString()); break;
                case 2: Assert.assertEquals("350-599", r.toString()); break;
                case 3: Assert.assertEquals("701-65535", r.toString()); break;
            }
            i++;
        }
    }

    @Test
    public void testRevert2()throws Exception{
        TreeSet<PortGenerator.Range> base = new TreeSet<>();
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(51, 99));
        ranges.add(new PortGenerator.Range(301, 349));
        ranges.add(new PortGenerator.Range(600, 65535));
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        Whitebox.invokeMethod(generator, "revert", base, ranges);
        Assert.assertEquals(3, base.size());

        int i = 0;
        for(PortGenerator.Range r : base){
            switch (i){
                case 0: Assert.assertEquals("1-50", r.toString()); break;
                case 1: Assert.assertEquals("100-300", r.toString()); break;
                case 2: Assert.assertEquals("350-599", r.toString()); break;
            }
            i++;
        }
    }

    @Test
    public void testFindSlotAtBegin()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(51, 99));
        ranges.add(new PortGenerator.Range(301, 349));
        ranges.add(new PortGenerator.Range(600, 65535));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertEquals(3, ranges.size());
        Assert.assertEquals("1-50", range.toString());
        Assert.assertEquals("1-99", ranges.first().toString());
    }

    @Test
    public void testFindSlotInBetween()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 99));
        ranges.add(new PortGenerator.Range(301, 349));
        ranges.add(new PortGenerator.Range(600, 65535));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertEquals(3, ranges.size());
        Assert.assertEquals("100-149", range.toString());
        Assert.assertEquals("1-149", ranges.first().toString());
    }

    @Test
    public void testFindSlotInBetweenWithMerge()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 99));
        ranges.add(new PortGenerator.Range(150, 349));
        ranges.add(new PortGenerator.Range(600, 65535));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertEquals(2, ranges.size());
        Assert.assertEquals("100-149", range.toString());
        Assert.assertEquals("1-349", ranges.first().toString());
    }

    @Test
    public void testFindSlotWithShortRange()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 99));
        ranges.add(new PortGenerator.Range(110, 65535));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertEquals(1, ranges.size());
        Assert.assertEquals("100-109", range.toString());
        Assert.assertEquals("1-65535", ranges.first().toString());
    }

    @Test
    public void testFindSlotAtLast()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 60000));

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertEquals(1, ranges.size());
        Assert.assertEquals("60001-60050", range.toString());
        Assert.assertEquals("1-60050", ranges.first().toString());
    }

    @Test
    public void testFindSlotAtFirstTime()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();

        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertEquals(1, ranges.size());
        Assert.assertEquals("1-50", range.toString());
        Assert.assertEquals("1-50", ranges.first().toString());
    }

    @Test
    public void testFindNoSlot()throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 65535));
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        PortGenerator.Range range = Whitebox.invokeMethod(generator, "findSlot", ranges);
        Assert.assertNull(range);
    }

    @Test
    public void testMergeSelf() throws Exception{
        TreeSet<PortGenerator.Range> ranges = new TreeSet<>();
        ranges.add(new PortGenerator.Range(1, 50));
        ranges.add(new PortGenerator.Range(51, 60));
        ranges.add(new PortGenerator.Range(100, 150));
        ranges.add(new PortGenerator.Range(151, 200));
        ranges.add(new PortGenerator.Range(210, 500));
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        TreeSet<PortGenerator.Range> newRanges = Whitebox.invokeMethod(generator, "mergeSelf", ranges);
        Assert.assertEquals(3, newRanges.size());

        int i = 0;
        for(PortGenerator.Range range : newRanges){
            switch (i){
                case 0: Assert.assertEquals("1-60", range.toString()); break;
                case 1: Assert.assertEquals("100-200", range.toString()); break;
                case 2: Assert.assertEquals("210-500", range.toString()); break;
            }
            i++;
        }
    }

    @Test
    public void testFindFreePort()throws Exception{
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        int port = generator.getFreePort();
        System.out.println(port);
    }

    @Test
    public void testReclaimPort()throws Exception{
        PortGenerator generator = Whitebox.invokeConstructor(PortGenerator.class);
        int port = generator.getFreePort();
        System.out.println(port);
        generator.reclaimPort(port);
        int batchSize = HpnlConfig.getInstance().getPortBatchSize();
        int i = 0;
        int finalPort = -1;
        while(i++ < batchSize){
            finalPort = generator.getFreePort();
        }
        Assert.assertEquals(port, finalPort);
    }

    @Test
    public void testFindFreePortUntilAllOccupied()throws Exception{
        if(!tmpDir.exists()){
            tmpDir.mkdirs();
        }
        portFile = new File(tmpDir, PortGenerator.FILE_NAME_PORTS);
        try(FileWriter writer = new FileWriter(portFile)){
            writer.write("1-50000\n");
            writer.write("50010-65535\n");
        }
        PortGenerator portGenerator = Whitebox.invokeConstructor(PortGenerator.class);
        int i = 0;
        int port = 0;
        int expected = 50001;
        while(i++<9){
            port = portGenerator.getFreePort();
            Assert.assertEquals(expected++, port);
        }
        Exception ee = null;
        try{
            portGenerator.getFreePort();
        }catch (RuntimeException e){
            ee = e;
        }
        Assert.assertNotNull(ee);
    }

    @Test
    public void testFindFreePortFromRecycled()throws Exception{
        if(!tmpDir.exists()){
            tmpDir.mkdirs();
        }
        portFile = new File(tmpDir, PortGenerator.FILE_NAME_PORTS);
        try(FileWriter writer = new FileWriter(portFile)){
            writer.write("1-65535\n");
        }
        createPidDir("0123", "40-44\n", "150-200\n");
        createPidDir("0124", "30-39\n", "400-410\n");
        PortGenerator portGenerator = Whitebox.invokeConstructor(PortGenerator.class);
        int i = 0;
        int port;
        int expected = 30;
        while(i++<15){
            port = portGenerator.getFreePort();
            Assert.assertEquals(expected++, port);
        }
        i = 0;
        expected = 150;
        while(i++<15){
            port = portGenerator.getFreePort();
            Assert.assertEquals(expected++, port);
        }
    }

    private void createPidDir(String pid, String recycledPorts, String assignedPorts)throws IOException {
        File pidDir = new File(tmpDir, PortGenerator.PID_PREFIX+pid);
        pidDir.mkdir();
        File recycled = new File(pidDir, PortGenerator.FILE_NAME_RECYCLED);
        try(FileWriter writer = new FileWriter(recycled)){
            writer.write(recycledPorts);
        }
        File assigned = new File(pidDir, PortGenerator.FILE_NAME_ASSIGNED);
        try(FileWriter writer = new FileWriter(assigned)){
            writer.write(assignedPorts);
        }
    }

    @After
    public void teardown()throws Exception{
        if(portFile != null){
            portFile.delete();
        }
        if(tmpDir != null){
            FileUtils.deleteDirectory(tmpDir);
        }
    }

    @AfterClass
    public static void afterClass(){
        Runtime.getRuntime().removeShutdownHook(PortGenerator.shutdownHook);
    }
}
