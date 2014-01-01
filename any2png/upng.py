#!/usr/bin/env python
# -*- coding:utf-8 -*-

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

# Copyright (C) 2011 Yaacov Zamir (2011) <kobi.zamir@gmail.com>
# Authors: 
#    Yaacov Zamir (2011) <kobi.zamir@gmail.com>

import sys
from struct import pack

from PySide.QtCore import Qt
from PySide.QtGui import QApplication
from PySide.QtGui import QFileDialog
from PySide.QtGui import QLabel, QImage, QPixmap, QPalette, QCursor, QPalette
from PySide.QtUiTools import QUiLoader

from png_enc import writeSPangImage, writePpm

def tr(string):
    return string

class UpngGui:
    def __init__(self):
        self.ui = None
        self.image = None
        
    def main(self, argv=None):
        
        if argv is None:
            argv = sys.argv

        self.app = QApplication(argv)

        loader = QUiLoader()
        self.ui = loader.load('./upng.ui')

        self.ui.openButton.clicked.connect(self.openImage)
        self.ui.resizeButton.clicked.connect(self.resizeImage)
        self.ui.saveButton.clicked.connect(self.saveImage)
        
        self.ui.show()
        
        return self.app.exec_()
    
    def openImage(self):
        
        fileName = QFileDialog.getOpenFileName(self.ui, 
            tr("Open Image"), "./", 
            tr("Image Files (*.png *.jpg *.bmp *.ppm)"))
        
        if fileName:
            fileName = fileName[0]
            
            self.image = QImage()
            self.image.load(fileName)
            w = self.image.width()
            h = self.image.height()
            
            self.ui.spinBoxWidth.setValue(w)
            self.ui.spinBoxHeight.setValue(h)
            
            self.resizeImage()
    
    def resizeImage(self):
        
        if not self.image:
            self.ui.scrollArea.takeWidget()
            return
        
        w = self.ui.spinBoxWidth.value()
        h = self.ui.spinBoxHeight.value()
        
        img = self.image.scaled(w,h)
        pix = QPixmap.fromImage(img)
        
        lbl = QLabel()
        lbl.setPixmap(pix)
        self.ui.scrollArea.takeWidget()
        self.ui.scrollArea.setWidget(lbl)
    
    def saveImage(self):
        
        if not self.image:
            return
        
        w = self.ui.spinBoxWidth.value()
        h = self.ui.spinBoxHeight.value()
        
        img = self.image.scaled(w,h).convertToFormat(QImage.Format_ARGB32)
        
        fileName = QFileDialog.getSaveFileName(self.ui, 
            tr("Save Simple Png Image"), "./", 
            tr("Image Files (*.png *.ppm)"))
        
        if fileName:
            fileName = fileName[0]
            
            self.app.setOverrideCursor(QCursor(Qt.WaitCursor))
            self.ui.setEnabled(False)
            
            if fileName.endswith('ppm'):
                writePpm(fileName, img)
            if fileName.endswith('png'):
                writeSPangImage(fileName, img, self.app)
            else:
                writeSPangImage(fileName + ".png", img, self.app)
            
            self.ui.setEnabled(True)
            self.app.restoreOverrideCursor()
            
if __name__ == '__main__':
    upng_gui = UpngGui()
    
    upng_gui.main()

