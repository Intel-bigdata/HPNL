package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlConfig;

import java.io.*;
import java.util.Queue;
import java.util.Random;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentLinkedQueue;

public class PortGenerator {

    private File dir;

    private File lockFile;

    private File portFile;

    private File recycleFile;

    private int portBatchSize;

    private Queue<Integer> queue = new ConcurrentLinkedQueue<>();

    private TreeSet<Range> ranges = new TreeSet<>();

    private static final String RECYCLE_PREFIX = "recycled-";

    private static final int MAX_PORT = 65535;

    private static final PortGenerator _INSTANCE = new PortGenerator();

    static{
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {_INSTANCE.release();}));
    }

    private PortGenerator(){
        String tmpDir = System.getProperty("java.io.tmpdir");
        String appId = HpnlConfig.getInstance().getAppId();
        File dir = new File(tmpDir+"/hpnl/"+appId);
        if(!dir.exists()){
            dir.mkdirs();
        }
        lockFile = new File(dir, ".lock");
        portFile = new File(dir, "ports");
        if(!portFile.exists()){
            try {
                portFile.createNewFile();
            }catch (IOException e){
                throw new RuntimeException("cannot create port file, "+portFile.getAbsolutePath(), e);
            }
        }
        //recycle
        Random random = new Random();
        int bound = 10000;
        int suffix = random.nextInt(bound);
        recycleFile = new File(dir, RECYCLE_PREFIX+suffix);
        try {
            while (!recycleFile.createNewFile()) {
                suffix = random.nextInt(bound);
                recycleFile = new File(dir, RECYCLE_PREFIX+suffix);
            }
        }catch (IOException e){
            throw new RuntimeException("cannot create port recycle file, "+recycleFile.getAbsolutePath(), e);
        }
        portBatchSize = HpnlConfig.getInstance().getPortBatchSize();
        if(portBatchSize > MAX_PORT){
            throw new RuntimeException("port batch size should be less than max port number, "+MAX_PORT);
        }
    }

    public static PortGenerator getInstance(){
        return _INSTANCE;
    }

    public int getFreePort(){
        Integer port = queue.poll();
        if(port == null){
            synchronized (this){
                port = queue.poll();
                if(port == null){
                    assignPorts();
                    port = queue.poll();
                }
            }
        }
        return port;
    }

    private void assignPorts(){
        putLock();
        File[] recycledFiles = null;
        try{
            //non-consecutive ranges
            TreeSet<Range> rangeSet = new TreeSet<>();
            getPortsFromPortFile(rangeSet);
            Range range = findSlot(rangeSet);
            if(range == null){//find ranges from recycled and merge them
                TreeSet<Range> recycledSet = new TreeSet<>();
                recycledFiles = getPortsFromRecycled(recycledSet);
                if(recycledSet.isEmpty()){
                    throw new RuntimeException("cannot find recycled ports whilst all other ports are occupied.");
                }
                //clear range of 1-65535 and get occupied port by reverting recycled
                revert(rangeSet, recycledSet);
            }
            //try again after recycling
            range = findSlot(rangeSet);
            if(range == null){
                throw new RuntimeException("all ports are occupied");
            }
            addRanges(range);
            writePortFile(rangeSet);
        }finally{
            removeLock();
            if(recycledFiles != null){
                for(File f : recycledFiles){
                    if(!f.delete()){
                        throw new RuntimeException("cannot delete recycled file, "+f.getAbsolutePath());
                    }
                }
            }
        }
    }

    private void addRanges(Range range){
        if(ranges.contains(range)){
            throw new RuntimeException("duplicate range assigned. "+range);
        }
        ranges.add(range);
        ranges = mergeSelf(ranges);
        for(int i=range.start; i<=range.end; i++){
            queue.add(i);
        }
    }

    private void revert(TreeSet<Range> rangeSet, TreeSet<Range> recycledSet){
        rangeSet.clear();
        Range p = null;
        for(Range r : recycledSet){
            if(p == null){
                if(r.start != 1){
                    rangeSet.add(new Range(1, r.start-1));
                }
                p = r;
                continue;
            }
            rangeSet.add(new Range(p.end+1, r.start-1));
            p = r;
        }
        if(p != null && p.end != MAX_PORT){
            rangeSet.add(new Range(p.end+1, MAX_PORT));
        }
    }

    private TreeSet<Range> mergeSelf(TreeSet<Range> rangeSet){
        if(rangeSet.size() <= 1){
            return rangeSet;
        }
        TreeSet<Range> retSet = new TreeSet<>();
        Range previous = null;
        for(Range range : rangeSet){
            if(previous == null){
                previous = range;
                retSet.add(range);
                continue;
            }
            if(previous.end >= range.start){
                throw new RuntimeException(String.format("previous range %s overlaps with current range %s",
                        previous, range));
            }
            if(previous.end == range.start - 1){//merge
                retSet.remove(previous);
                previous = new Range(previous.start, range.end);
                retSet.add(previous);
            }else {
                previous = range;
                retSet.add(range);
            }
        }
        return retSet;
    }

    private void writePortFile(TreeSet<Range> rangeSet){
        writeToFile(rangeSet, portFile);
    }

    private void writeToFile(TreeSet<Range> rangeSet, File file) {
        try(FileWriter writer = new FileWriter(file)){
            for(Range r : rangeSet){
                writer.write(r.toString());
                writer.write('\n');
            }
        } catch (IOException e) {
            throw new RuntimeException("failed to write port ranges to file, "+file.getAbsolutePath(), e);
        }
    }

    private Range findSlot(TreeSet<Range> rangeSet){
        Range p = null;
        Range current = null;
        Range result;
        int start = 0, end = 0;
        for(Range r : rangeSet){
            if(p == null){
                if(r.start > 1) {//from 1
                    start = 1;
                    end = r.start - 1;
                    end = portBatchSize >= end ? end : portBatchSize;
                    current = r;
                    break;
                }
                p = r;
                continue;
            }
            //after first range
            start = p.end + 1;
            int size = r.start - p.end - 1;
            end = portBatchSize >= size ? r.start-1 : p.end+portBatchSize;
            current = r;
            break;
        }
        if(start > 0){
            result = new Range(start, end);
            boolean needMerge = result.end == current.start - 1;
            if(p == null){
                if(needMerge) {
                    rangeSet.remove(current);
                }
                rangeSet.add(needMerge ? new Range(1, current.end) : result);
            }else {
                rangeSet.remove(p);
                if (needMerge) {
                    rangeSet.remove(current);
                }
                rangeSet.add(new Range(p.start, needMerge ? current.end : result.end));
            }
            return result;
        }

        if(p == null){//empty rangeSet
            result = new Range(1, portBatchSize);
            rangeSet.add(result);
            return result;
        }
        //only one existing range starting from 1
        if(p.end < MAX_PORT){
            end = p.end + portBatchSize;
            end = end > MAX_PORT ? MAX_PORT : end;
            result = new Range(p.end+1, end);
            rangeSet.remove(p);
            rangeSet.add(new Range(p.start, end));
            return result;
        }
        return null;
    }

    private void putLock(){
        try {
            while (!lockFile.createNewFile()) {
                Thread.sleep(50);
            }
        }catch (IOException | InterruptedException e){
            throw new RuntimeException("failed to create lock file, "+lockFile.getAbsolutePath(), e);
        }
    }

    private void removeLock(){
        try{
            if(!lockFile.delete()){
                Thread.sleep(10);
                if(!lockFile.delete()){
                    throw new RuntimeException("cannot delete lock file, "+lockFile.getAbsolutePath());
                }
            }
        }catch (InterruptedException e){
            if(!lockFile.delete()){
                throw new RuntimeException("cannot delete lock file after interruption, "+lockFile.getAbsolutePath(), e);
            }
        }
    }

    private TreeSet<Range> getPortsFromPortFile(TreeSet<Range> rangeSet){
        rangeSet.addAll(getPortsFromFile(portFile));
        return rangeSet;
    }

    private File[] getPortsFromRecycled(TreeSet<Range> rangeSet){
        File[] recycledFiles = dir.listFiles((f, name) -> name.startsWith(RECYCLE_PREFIX));
        if(recycledFiles == null){
            return null;
        }
        for(File f : recycledFiles){
            TreeSet<Range> fileRanges = getPortsFromFile(f);
            mergeRange(rangeSet, fileRanges);
        }
        return recycledFiles;
    }

    private void mergeRange(TreeSet<Range> rangeSet, TreeSet<Range> fileRanges){
        if(rangeSet.isEmpty()){
            rangeSet.addAll(fileRanges);
            return;
        }
        for(Range r : fileRanges){
            //merge with previous range
            Range p = rangeSet.lower(r);
            Range newRange = r;
            if(p != null){
                if(p.end >= r.start){
                    throw new IllegalStateException(
                            String.format("this range's end (%d) should be less than " +
                                    "next range's start (%d).", p.end, r.start));
                }
                if(p.end == r.start-1){
                    newRange = new Range(p.start, r.end);
                    rangeSet.remove(p);
                    rangeSet.add(newRange);
                }
            }
            //merge with next range
            Range n = rangeSet.higher(newRange);
            if(n != null){
                if(n.start <= newRange.end){
                    throw new IllegalStateException(
                            String.format("this range's start (%d) should be greater than " +
                                    "previous range's end (%d).", n.start, newRange.end));
                }
                if(n.start == newRange.end+1){
                    Range tmp = new Range(newRange.start, n.end);
                    rangeSet.remove(newRange);
                    rangeSet.remove(n);
                    rangeSet.add(tmp);
                    newRange = tmp;
                }
            }
            if(newRange == r){//not merged, add it
                rangeSet.add(r);
            }
        }
    }

    /**
     * non-consecutive ranges
     * @param f
     * @return
     */
    private TreeSet<Range> getPortsFromFile(File f){
        TreeSet<Range> rangeSet = new TreeSet<>();
        try(BufferedReader reader = new BufferedReader(new FileReader(f))){
            String line;
            while((line=reader.readLine()) != null){
                if(line.length() > 0){
                    String[] st = line.split("-");
                    Range range = new Range(st[0], st[1]);
                    if(!rangeSet.isEmpty()) {
                        Range last = rangeSet.last();
                        if (range.start <= last.end) {
                            throw new IllegalStateException(
                                    String.format("this range's start (%d) should be greater than " +
                                            "last range's end (%d).", range.start, last.end));
                        }
                        if(range.start == last.end + 1){
                            rangeSet.remove(last);
                            rangeSet.add(new Range(last.start, range.end));
                            continue;
                        }
                    }

                    rangeSet.add(range);
                }
            }
        }catch (IOException e){
            throw new RuntimeException("error when get ports from file "+f.getAbsolutePath(), e);
        }
        return rangeSet;
    }

    public void reclaimPort(int port){
        queue.add(port);
    }

    public void release(){
        if(recycleFile.exists()) {
            writeToFile(ranges, recycleFile);
        }
    }

    protected static class Range implements Comparable<Range>{
        private final int start;
        private final int end;
        public Range(int start, int end){
            this.start = start;
            this.end = end;
            if(end < start){
                throw new IllegalStateException(
                        String.format("end (%d) should be no less than start (%d).", end, start));
            }
            if(start < 1){
                throw new IllegalStateException(
                        String.format("start should be no less than 1", start));
            }
            if(end > MAX_PORT){
                throw new IllegalStateException(
                        String.format("end should be no greater than "+MAX_PORT, end));
            }
        }

        public Range(String start, String end){
            this(Integer.valueOf(start).intValue(), Integer.valueOf(end).intValue());
        }

        @Override
        public int compareTo(Range range) {
            return start - range.start;
        }

        @Override
        public boolean equals(Object o){
            if(!(o instanceof Range)){
                return false;
            }
            Range other = (Range)o;
            return start == other.start && end == other.end;
        }

        @Override
        public int hashCode(){
            return start + end;
        }

        @Override
        public String toString(){
            return new StringBuffer(String.valueOf(start)).append('-').append(end).toString();
        }
    }
}
