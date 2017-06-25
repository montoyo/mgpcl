package net.mgpcl.netlogger;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import java.awt.*;

public class LogCellRenderer extends DefaultTableCellRenderer {

    private Font normalFont = null;
    private Font italicFont = null;

    @Override
    public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
        LogTableModel mdl = (LogTableModel) table.getModel();
        Component c = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);

        if(normalFont == null || italicFont == null) {
            normalFont = c.getFont();
            italicFont = normalFont.deriveFont(Font.ITALIC);
        }

        Color color = mdl.getColorForRow(row);
		if(color.equals(Color.white)) {
			//Don't change if it's white; use default background color
			c.setBackground(null);
			c.setForeground(null);
		} else {
	        c.setBackground(color); 

			if(color.equals(Color.red))
				c.setForeground(Color.white);
			else
				c.setForeground(Color.black);
		}

        if(mdl.isRowItalic(row))
            c.setFont(italicFont);
        else
            c.setFont(normalFont);

        return c;
    }

}
