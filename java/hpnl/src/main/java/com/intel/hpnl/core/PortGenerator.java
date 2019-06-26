package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlConfig;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

public class PortGenerator {

    private File appDir;

    private File dir;

    private File globalLockFile;

    private File globalAssignedPortFile;

    private File recycledPortFile;

    private File assignedPortFile;

    private int portBatchSize;

    private Queue<Integer> queue = new ConcurrentLinkedQueue<>();

    private TreeSet<Range> ranges = new TreeSet<>();

    private int pid;

    public static final String FILE_NAME_LOCK = "lock";
    public static final String FILE_NAME_PORTS = "ports";
    public static final String FILE_NAME_RECYCLED = "recycled";
    public static final String FILE_NAME_ASSIGNED = "assigned";
    public static final String PID_PREFIX = "pid-";

    private static final int MAX_PORT = 65535;

    private static final PortGenerator _INSTANCE = new PortGenerator();

    private static final Logger log = LoggerFactory.getLogger(PortGenerator.class);

    protected static final Thread shutdownHook = new Thread(() -> _INSTANCE.release());

    static{
        Runtime.getRuntime().addShutdownHook(shutdownHook);
    }

    public static PortGenerator getInstance(){
        return _INSTANCE;
    }

    private PortGenerator(){
        String tmpDir = System.getProperty("java.io.tmpdir");
        String appId = HpnlConfig.getInstance().getAppId();
        pid = Utils.getPid();
        appDir = new File(tmpDir, "hpnl/"+ appId);
        if(!appDir.exists()){
            appDir.mkdirs();
        }
        globalLockFile = new File(appDir, FILE_NAME_LOCK);
        createGlobalPortFile(appDir);
        //own folder
        createProcessFolder(appDir, pid, globalLockFile, globalAssignedPortFile);

        try {
            recycledPortFile = new File(dir, FILE_NAME_RECYCLED);
            recycledPortFile.createNewFile();
            assignedPortFile = new File(dir, FILE_NAME_ASSIGNED);
            assignedPortFile.createNewFile();
        }catch (IOException e){
            throw new RuntimeException("failed to create recycled file or assigned file", e);
        }

        portBatchSize = HpnlConfig.getInstance().getPortBatchSize();
        if(portBatchSize > MAX_PORT){
            throw new RuntimeException("port batch size should be less than max port number, "+MAX_PORT);
        }
    }

    private void createGlobalPortFile(File appDir) {
        globalAssignedPortFile = new File(appDir, FILE_NAME_PORTS);
        if (globalAssignedPortFile.exists()) {
            return;
        }
        putLock();
        try {
            if (globalAssignedPortFile.exists()) {
                return;
            }
            try {
                globalAssignedPortFile.createNewFile();
            } catch (IOException e) {
                throw new RuntimeException("cannot create port file, " + globalAssignedPortFile.getAbsolutePath(), e);
            }
        }finally {
            removeLock();
        }
    }

    private void createProcessFolder(File appDir, int pid, File globalLockFile, File globalPortFile) {
        dir = new File(appDir, PID_PREFIX + pid);
        putLock();
        try {
            if (dir.exists()) {
                recyclePorts(dir);
                deleteDir(dir);
            }
            dir.mkdir();
        }finally {
            removeLock();
        }
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

    private void deleteDir(File dir) {
        try {
            FileUtils.deleteDirectory(dir);
        }catch (IOException e){
            throw new RuntimeException("failed to delete old dir, "+dir.getAbsolutePath(), e);
        }
    }

    /**
     * recycle ports from old died process
     * @param dir
     */
    private void recyclePorts(File dir) {
        TreeSet<Range> recycledRanges = null;
        File recycled = new File(dir, FILE_NAME_RECYCLED);
        if(recycled.exists() && recycled.length() > 0){
            recycledRanges = getPortsFromFile(recycled);
        }
        File assigned = new File(dir, FILE_NAME_ASSIGNED);
        if(assigned.exists() && assigned.length() > 0){
            TreeSet<Range> assignedRanges = getPortsFromFile(assigned);
            if(recycledRanges == null){
                recycledRanges = assignedRanges;
            }else{
                mergeRange(recycledRanges, assignedRanges);
            }
        }
        if(recycledRanges != null && !recycledRanges.isEmpty()){
            TreeSet<Range> allAssignedRanges = getPortsFromGlobalAssignedPortFile();
            for(Range r : recycledRanges){
                Range containingRange = allAssignedRanges.floor(r);
                if(containingRange == null){
                    throw new IllegalStateException(
                            String.format("there should be ranges in %s to cover assigned ports under %s",
                                    globalAssignedPortFile.getAbsolutePath(), dir.getAbsolutePath()));
                }
                if(r.end > containingRange.end){
                    throw new IllegalStateException(
                            String.format("containing range (%s) should cover assigned ports (%s) under %s",
                                    containingRange.toString(), r.toString(), dir.getAbsolutePath()));
                }
                allAssignedRanges.remove(containingRange);
                if(r.start == containingRange.start && r.end < containingRange.end){
                    allAssignedRanges.add(new Range(r.end+1, containingRange.end));
                    continue;
                }
                //r.start > containingRange.start
                allAssignedRanges.add(new Range(containingRange.start, r.start - 1));
                if(r.end < containingRange.end){
                    allAssignedRanges.add(new Range(r.end+1, containingRange.end));
                }
            }
            writeGlobalAssignedPortFile(allAssignedRanges);
        }
    }

    private void assignPorts(){
        putLock();
        List<File> recycledFiles = null;
        try{
            //non-consecutive ranges
            TreeSet<Range> allAssignedRanges = getPortsFromGlobalAssignedPortFile();
            Range range = findSlot(allAssignedRanges);
            if(range == null){//find ranges from recycled and merge them
                if(allAssignedRanges.size() != 1){
                    throw new RuntimeException("there should be only one range 1-65535");
                }
                TreeSet<Range> recycledSet = new TreeSet<>();
                recycledFiles = getPortsFromRecycled(recycledSet);
                if(recycledSet.isEmpty()){
                    throw new RuntimeException("cannot find recycled ports from died process");
                }
                //clear range of 1-65535 and get occupied port by reverting recycled
                revert(allAssignedRanges, recycledSet);
                //try again after recycling
                range = findSlot(allAssignedRanges);
                if(range == null){
                    throw new RuntimeException("all ports are occupied");
                }
            }
            addRanges(range);
            writeToFile(ranges, assignedPortFile);
            writeGlobalAssignedPortFile(allAssignedRanges);
        }finally{
            removeLock();
            if(recycledFiles != null){
                for(File f : recycledFiles){
                    if(f.isFile()) {
                        if (!f.delete()) {
                            throw new RuntimeException("cannot delete recycled file, " + f.getAbsolutePath());
                        }
                    }else{
                        try {
                            FileUtils.deleteDirectory(f);
                        }catch (IOException e){
                            throw new RuntimeException("cannot delete dir, " + f.getAbsolutePath());
                        }
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

    private void writeGlobalAssignedPortFile(TreeSet<Range> rangeSet){
        writeToFile(rangeSet, globalAssignedPortFile);
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
            while (!globalLockFile.createNewFile()) {
                Thread.sleep(10);
            }
        }catch (IOException | InterruptedException e){
            throw new RuntimeException("failed to create lock file, "+globalLockFile.getAbsolutePath(), e);
        }
    }

    private void removeLock(){
        try{
            if(!globalLockFile.delete()){
                Thread.sleep(10);
                if(!globalLockFile.delete()){
                    throw new RuntimeException("cannot delete lock file, "+globalLockFile.getAbsolutePath());
                }
            }
        }catch (InterruptedException e){
            if(!globalLockFile.delete()){
                throw new RuntimeException("cannot delete lock file after interruption, "+globalLockFile.getAbsolutePath(), e);
            }
        }
    }

    private TreeSet<Range> getPortsFromGlobalAssignedPortFile(){
        return getPortsFromFile(globalAssignedPortFile);
    }

    /**
     * get ports from both recycled, and assigned of died process
     * @param rangeSet
     * @return
     */
    private List<File> getPortsFromRecycled(TreeSet<Range> rangeSet){
        File[] processDirs = appDir.listFiles((f, name) -> name.startsWith(PID_PREFIX));

        if(processDirs == null || processDirs.length == 0){
            return null;
        }
        Set<Integer> javaProcesses = getJavaProcesses();
        List<File> recycledFiles = new ArrayList<>();
        for(File pdir : processDirs){
            recycledFiles.add(new File(pdir, FILE_NAME_RECYCLED));
            int start = pdir.getName().indexOf(PID_PREFIX) + 4;
            int pid = Integer.valueOf(pdir.getName().substring(start));
            if(!javaProcesses.contains(pid)){
                recycledFiles.add(new File(pdir, FILE_NAME_ASSIGNED));
                recycledFiles.add(pdir);
            }
        }
        for(File f : recycledFiles){
            if(f.isFile()) {
                TreeSet<Range> fileRanges = getPortsFromFile(f);
                mergeRange(rangeSet, fileRanges);
            }
        }
        return recycledFiles;
    }

    private Set<Integer> getJavaProcesses() {
        Set<Integer> pids = new HashSet<>();
        List<String> commands = new ArrayList<>();
        commands.add("/bin/bash");
        commands.add("-c");
        commands.add("pgrep -f java");
        ProcessBuilder pb = new ProcessBuilder(commands);
        try {
            Process shell = pb.start();
            try(BufferedReader reader = new BufferedReader(new InputStreamReader(shell.getInputStream()))){
                String line;
                while((line=reader.readLine()) != null){
                    pids.add(Integer.valueOf(line.trim()));
                }
            }
            int status = shell.waitFor();
            log.info("exit result of get java processes shell. "+status);
        }catch (IOException | InterruptedException e){
            throw new RuntimeException("cannot run shell command to get running java processes. "+commands.toString(), e);
        }
        return pids;
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
        writeToFile(ranges, recycledPortFile);
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
