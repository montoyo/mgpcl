package net.mgpcl.netlogger;

import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.datatransfer.StringSelection;
import java.awt.event.*;
import java.io.*;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicBoolean;

public class NetLogger implements WindowListener, ActionListener {

    public static NetLogger INSTANCE;

    private Thread thread;
    private AtomicBoolean running = new AtomicBoolean(false);
    private ServerSocket sock;

    private final Object clientLock = new Object();
    private Socket client;
    private OutputStream sockOut;

    private JFrame window;
    private JTable table;
    private LogTableModel tableContent;
    private JCheckBox levels[] = new JCheckBox[4];
    private JCheckBox autoScroll;
    private JButton filterButton;
    private JScrollPane scrollPane;
    private boolean closeOnDisconnect = false;
    private int oldHorizontalScroll = 0;

    private class CheckBoxChanger implements MouseListener {

        private JCheckBox cb;

        CheckBoxChanger(JCheckBox cb) {
            this.cb = cb;
        }

        @Override
        public void mouseClicked(MouseEvent e) {
            cb.setSelected(!cb.isSelected());
            actionPerformed(new ActionEvent(cb, 0, null));
        }

        @Override
        public void mousePressed(MouseEvent e) {
        }

        @Override
        public void mouseReleased(MouseEvent e) {
        }

        @Override
        public void mouseEntered(MouseEvent e) {
        }

        @Override
        public void mouseExited(MouseEvent e) {
        }

    }

    public static void main(String[] args)
    {
        int port = 1234;
        boolean cod = false;

        for(int i = 0; i < args.length; i++) {
            if(args[i].equalsIgnoreCase("--port")) {
                if(i + 1 >= args.length) {
                    System.err.println("Missing --port value");
                    System.exit(-1);
                    return;
                }

                try {
                    port = Integer.parseInt(args[i + 1]);
                } catch(NumberFormatException ex) {
                    System.err.println("Invalid --port format");
                    System.exit(-1);
                    return;
                }

                if(port < 0 || port >= 65536) {
                    System.err.println("Invalid --port value");
                    System.exit(-1);
                    return;
                }
            } else if(args[i].equalsIgnoreCase("--close-on-disconnect"))
                cod = true;
            else
                System.out.println("Note: Unrecognized argument " + args[i]);
        }

        System.out.println("Starting NetLogger on *:" + port);
        INSTANCE = new NetLogger(port);
        INSTANCE.closeOnDisconnect = cod;
        INSTANCE.thread = new Thread(() -> INSTANCE.listen());

        try {
            Thread.sleep(1000); //Java GUI init is slow...
        } catch(Throwable t) {}

        INSTANCE.thread.start();
    }

    NetLogger(int port)
    {
        try {
            sock = new ServerSocket(port);
        } catch(IOException ex) {
            System.err.println("Could NOT start server:");
            ex.printStackTrace();
            System.exit(-2);
            return;
        }

        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch(Throwable t) {}

        window = new JFrame("MGPCL's NetLogger");
        window.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        window.addWindowListener(this);
        window.setSize(750, 350);

        JPanel pane = new JPanel();
        pane.setLayout(new GridBagLayout());
        pane.setBorder(new EmptyBorder(3, 3, 3, 3));

        tableContent = new LogTableModel();
        table = new JTable(this.tableContent);
        scrollPane = new JScrollPane(table);

        table.setFillsViewportHeight(true);
        table.setDefaultRenderer(Object.class, new LogCellRenderer());
        table.setShowGrid(false);

        scrollPane.getVerticalScrollBar().addAdjustmentListener(e -> {
            if(autoScroll.isSelected()) {
                if(e.getValueIsAdjusting())
                    autoScroll.setSelected(false);
                else
                    e.getAdjustable().setValue(e.getAdjustable().getMaximum());
            }
        });

        scrollPane.getHorizontalScrollBar().addAdjustmentListener(e -> {
            if(e.getValueIsAdjusting())
                oldHorizontalScroll = e.getValue();
            else
                e.getAdjustable().setValue(oldHorizontalScroll);
        });

        scrollPane.addMouseWheelListener(e -> {
            if(autoScroll.isSelected()) {
                autoScroll.setSelected(false);

                /* //FIXME: The first scroll is canceled... sadly the fix below doesn't work
                JScrollBar bar = scrollPane.getVerticalScrollBar();
                int mul = e.getWheelRotation() / Math.abs(e.getWheelRotation());

                if(e.getScrollType() == MouseWheelEvent.WHEEL_BLOCK_SCROLL)
                    bar.setValue(bar.getValue() + mul * bar.getBlockIncrement());
                else
                    bar.setValue(bar.getValue() + mul * bar.getUnitIncrement());
                 */
            }
        });

        JPopupMenu menu = new JPopupMenu();
        JMenuItem copyCellItem = new JMenuItem("Copy cell");
        JMenuItem copyLineItem = new JMenuItem("Copy whole line");

        copyCellItem.addActionListener(e -> {
            int x = table.getSelectedColumn();
            int y = table.getSelectedRow();

            if(x >= 0 && x < tableContent.getColumnCount() && y >= 0 && y < tableContent.getRowCount())
                Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection((String) tableContent.getValueAt(y, x)), null);
        });

        copyLineItem.addActionListener(e -> {
            int y = table.getSelectedRow();

            if(y >= 0 && y < tableContent.getRowCount())
                Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(tableContent.rowToString(y)), null);
        });

        menu.add(copyCellItem);
        menu.add(copyLineItem);
        table.setComponentPopupMenu(menu);

        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.BOTH;
        c.weightx = 1.0;
        c.weighty = 1.0;
        c.gridwidth = levels.length + 2;
        c.insets = new Insets(0, 0, 2, 0);

        pane.add(scrollPane, c);
        createLevelCheckbox(pane, 0, "Debug");
        createLevelCheckbox(pane, 1, "Info");
        createLevelCheckbox(pane, 2, "Warning");
        createLevelCheckbox(pane, 3, "Error");

        autoScroll = new JCheckBox();
        autoScroll.setSelected(true);
        autoScroll.addActionListener(this);
        putCheckbox(pane, levels.length, autoScroll, null, "Scroll");

        filterButton = new JButton("Filter...");
        filterButton.addActionListener(this);

        c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.gridx = levels.length + 1;
        c.gridy = 1;
        c.weightx = 2.0 / 6.0;

        pane.add(filterButton, c);

        window.setContentPane(pane);
        window.setVisible(true);

        table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        table.getColumnModel().getColumn(0).setPreferredWidth(50);
        table.getColumnModel().getColumn(1).setPreferredWidth(50);
        table.getColumnModel().getColumn(2).setPreferredWidth(100);
        table.getColumnModel().getColumn(3).setPreferredWidth(40);
        //table.getColumnModel().getColumn(4).setPreferredWidth(table.getWidth() - (50 + 50 + 100 + 40));
        table.getColumnModel().getColumn(4).setPreferredWidth(1000);
    }

    private void createLevelCheckbox(JPanel pane, int x, String name)
    {
        Icon ico = null;
        try {
            ico = new ImageIcon(ImageIO.read(NetLogger.class.getResourceAsStream("/icons/" + name.toLowerCase() + ".png")));
        } catch(Throwable t) {
            t.printStackTrace();
        }

        levels[x] = new JCheckBox();
        levels[x].setSelected(true);
        levels[x].addActionListener(this);

        putCheckbox(pane, x, levels[x], ico, name);
    }

    private void putCheckbox(JPanel pane, int xPos, JCheckBox cb, Icon icon, String label)
    {
        JLabel lbl;
        if(icon == null)
            lbl = new JLabel(label, JLabel.LEFT);
        else
            lbl = new JLabel(label, icon, JLabel.LEFT);

        lbl.addMouseListener(new CheckBoxChanger(cb));

        JPanel inner = new JPanel();
        inner.setLayout(new GridBagLayout());

        GridBagConstraints c = new GridBagConstraints();
        c.gridx = 0;
        inner.add(cb, c);

        c = new GridBagConstraints();
        c.gridx = 1;
        inner.add(lbl, c);

        c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.weightx = 1.0;
        c.gridx = 2;
        inner.add(Box.createHorizontalBox(), c);

        c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.weightx = 1.0 / 6.0;
        c.gridx = xPos;
        c.gridy = 1;

        pane.add(inner, c);
    }

    private void listen()
    {
        running.set(true);

        while(running.get()) {
            Socket cli = null;

            while(cli == null && running.get()) {
                try {
                    System.out.println("Waiting for client...");
                    cli = sock.accept();
                } catch(IOException ex) {
                    if(running.get())
                        ex.printStackTrace();
                }
            }

            if(cli != null) {
                synchronized(clientLock) {
                    client = cli;
                }

                InetAddress addr = client.getInetAddress();
                if(addr != null)
                    System.out.println("Got client from: " + addr.getHostAddress() + ":" + client.getPort());

                InputStream sockIn;
                try {
                    sockIn = client.getInputStream();
                    sockOut = client.getOutputStream();
                } catch(IOException ex) {
                    ex.printStackTrace();
                    System.exit(-3);
                    return; //Shut the warning up
                }

                //Send current config (disabled levels)
                SwingUtilities.invokeLater(() -> {
                    tableContent.clear();

                    for(int i = 0; i < levels.length; i++) {
                        if(!levels[i].isSelected())
                            sendLevel(false, i);
                    }
                });

                //Receive packets
                boolean normalExit = handlePackets(sockIn);

                synchronized(clientLock) {
                    client = null;
                    sockOut = null;
                }

                //Make sure the client is closed
                try {
                    cli.close();
                } catch(Throwable t) {}

                //Close on disconnect
                if(normalExit && closeOnDisconnect) {
                    try {
                        Thread.sleep(1000);
                    } catch(Throwable t) {}

                    SwingUtilities.invokeLater(() -> windowClosing(null));
                    return;
                }
            }
        }
    }

    private boolean handlePackets(InputStream sockIn)
    {
        try {
            DataInputStream dis = new DataInputStream(sockIn);

            while(running.get()) {
                int pktSize = dis.readInt();

                if(pktSize < 4 || pktSize > 8192)
                    System.out.println("Spotted invalid packet with size " + pktSize);
                else {
                    pktSize -= 4;
                    byte[] pkt = new byte[pktSize];
                    int offset = 0;

                    while(pktSize > 0) {
                        int rd = dis.read(pkt, offset, pktSize);
                        if(rd == 0)
                            throw new EOFException();

                        offset += rd;
                        pktSize -= rd;
                    }

                    handlePacket(new DataInputStream(new ByteArrayInputStream(pkt)));
                }
            }
        } catch(EOFException ex) {
            try {
                client.close();
            } catch(Throwable t) {}

            client = null;
            SwingUtilities.invokeLater(() -> tableContent.addLine(new LogTableModel.LogRow()));
            return true; //Client disconnected normally
        } catch(IOException ex) {
            if(running.get())
                ex.printStackTrace();
        }

        return false;
    }

    private void handlePacket(DataInputStream dis)
    {
        SwingUtilities.invokeLater(() -> {
            try {
                tableContent.addLine(new LogTableModel.LogRow(dis));
            } catch(IOException ex) {
                ex.printStackTrace();
            }
        });
    }

    public void sendPacket(byte[] data)
    {
        byte[] pkt = new byte[data.length + 4];
        pkt[0] = (byte) ((pkt.length & 0xFF000000) >> 24);
        pkt[1] = (byte) ((pkt.length & 0x00FF0000) >> 16);
        pkt[2] = (byte) ((pkt.length & 0x0000FF00) >> 8);
        pkt[3] = (byte) (pkt.length & 0x000000FF);

        System.arraycopy(data, 0, pkt, 4, data.length);

        synchronized(clientLock) {
            if(sockOut == null) {
                System.out.println("!!! -> Can't send packet; client is disconnected...");
                return;
            }

            try {
                sockOut.write(pkt);
            } catch(IOException ex) {
                ex.printStackTrace();
            }
        }
    }

    public void sendLevel(boolean enabled, int lvl)
    {
        byte[] data = new byte[2];
        data[0] = (byte) (enabled ? 0xEE : 0xDD);
        data[1] = (byte) lvl;

        sendPacket(data);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        for(int i = 0; i < levels.length; i++) {
            if(e.getSource() == levels[i]) {
                sendLevel(levels[i].isSelected(), i);
                break;
            }
        }
    }

    @Override
    public void windowOpened(WindowEvent e) {
    }

    @Override
    public void windowClosing(WindowEvent ev) {
        System.out.println("Shutting down...");
        running.set(false);

        //End client's connection (if any)
        synchronized(clientLock) {
            if(client != null) {
                try {
                    client.close();
                } catch(Throwable t) {}
            }
        }

        //Close server
        try {
            sock.close();
        } catch(Throwable t) {}

        //Wait for thread to end
        try {
            thread.join();
        } catch(InterruptedException ex) {}

        //Finally, close dat window
        window.dispose();
    }

    @Override
    public void windowClosed(WindowEvent e) {
    }

    @Override
    public void windowIconified(WindowEvent e) {
    }

    @Override
    public void windowDeiconified(WindowEvent e) {
    }

    @Override
    public void windowActivated(WindowEvent e) {
    }

    @Override
    public void windowDeactivated(WindowEvent e) {
    }
}
