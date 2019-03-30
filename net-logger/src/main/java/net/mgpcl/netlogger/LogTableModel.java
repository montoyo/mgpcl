package net.mgpcl.netlogger;

import javax.swing.table.AbstractTableModel;
import java.awt.*;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;

public class LogTableModel extends AbstractTableModel {

    public static class LogRow {

        public byte level;
        public String cols[] = new String[5];

        private static String readString(DataInputStream dis) throws IOException
        {
            byte[] data = new byte[dis.readShort()];
            dis.read(data);

            return new String(data, "UTF-8");
        }

        public LogRow(DataInputStream dis) throws IOException
        {
            level = dis.readByte();
            if(level == 0)
                cols[0] = "Debug";
            else if(level == 1)
                cols[0] = "Info";
            else if(level == 2)
                cols[0] = "Warning";
            else if(level == 3)
                cols[0] = "Error";
            else
                cols[0] = "???";

            cols[1] = readString(dis);
            cols[2] = readString(dis);
            cols[3] = "" + dis.readShort();
            cols[4] = readString(dis);
        }

        public LogRow()
        {
            level = -1;
            for(int i = 0; i < 4; i++)
                cols[i] = "-";

            cols[4] = "Client disconnected.";
        }

    }

    private static final String[] cols = new String[] { "Level", "Thread", "File", "Line", "Message" };
    private ArrayList<LogRow> rows = new ArrayList<LogRow>();

    public void addLine(LogRow r)
    {
        rows.add(r);
        fireTableDataChanged();
    }

    public void clear()
    {
        rows.clear();
        fireTableDataChanged();
    }

    public Color getColorForRow(int r, boolean sel)
    {
        byte l = rows.get(r).level;
        if(l == 0 || l == -1)
            return sel ? Color.lightGray : Color.white;
        else if(l == 2)
            return sel ? Color.yellow : Color.orange;
        else if(l == 3)
            return sel ? new Color(255, 50, 50) : Color.red;

        return sel ? new Color(83, 225, 255) : new Color(57, 185, 255);
    }

    public boolean isRowItalic(int r) {
        return rows.get(r).level == -1;
    }

    @Override
    public int getRowCount() {
        return rows.size();
    }

    @Override
    public int getColumnCount() {
        return 5;
    }

    @Override
    public String getColumnName(int i) {
        return cols[i];
    }

    @Override
    public Object getValueAt(int rowIndex, int columnIndex) {
        return rows.get(rowIndex).cols[columnIndex];
    }

    public String rowToString(int y) {
        final String[] r = rows.get(y).cols;
        return (new StringBuilder()).append('[').append(r[0]).append("] [").append(r[1]).append("] [").append(r[2]).append(':').append(r[3]).append("] ").append(r[4]).toString();
    }

}
