/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package editor.gui;

import common.Util;
import editor.Main;
import java.awt.Image;
import java.awt.image.AbstractMultiResolutionImage;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

/** A customized multi-resolution image with known heights
 *
 * @author elvio
 */
class CustomMultiResolutionImage extends AbstractMultiResolutionImage {
    
    List<Image> images = new LinkedList<>();
    int[] srcHeights;
    String[] srcImages;
    Image[] loaded;

    public CustomMultiResolutionImage(int[] srcHeights, String[] srcImages) {
        this.srcHeights = srcHeights;
        this.srcImages = srcImages;
        this.loaded = new Image[this.srcImages.length];
//        this.images.add(new StubImage(srcHeights[0], srcHeights[0]));
    }

    @Override
    protected Image getBaseImage() {
//        return images.get(0); // the stub
        return getResolutionVariant(srcHeights[0], srcHeights[0]);
    }

    @Override
    public Image getResolutionVariant(double destImageWidth, double destImageHeight) {
        int targetHeight = (int) destImageHeight;
        for (Image img : images) {
            if (//!(img instanceof StubImage) && 
                    img.getHeight(null) == targetHeight) 
            {
                return img;
            }
        }
        // Image does not exists. load the closets, resize, store and return
        int sel = 0;
        //            int j = srcSizes.length - 1;
        //            while (j >= 0 && srcSizes[j] >= targetWidth) {
        //                sel = j;
        //                j--;
        //            }
        // Select the smallest that fits the target width
        int j = 0;
        while (j < srcHeights.length) {
            if (srcHeights[j] <= targetHeight && srcHeights[j] > srcHeights[sel]) {
                sel = j;
            }
            j++;
        }
        // Load image i
        if (loaded[sel] == null) {
            loaded[sel] = Util.loadImage(srcImages[sel]);
            images.add(loaded[sel]);
        }
        Image img = loaded[sel];
        // Resize image if needed
        if (srcHeights[sel] != targetHeight) {
            try {
                System.out.println("RESIZE " + srcHeights[sel] + " into " + targetHeight + " " + srcImages[sel]);
                img = resizeCenterImage(loaded[sel], (int) destImageWidth, targetHeight);
                images.add(img);
            } catch (IOException e) {
                Main.logException(e, true);
            }
        }
        return img;
    }

    //        BufferedImage resizeImage(Image originalImage, int targetWidth, int targetHeight) throws IOException {
    //            Image resultingImage = originalImage.getScaledInstance(targetWidth, targetHeight, Image.SCALE_DEFAULT);
    //            BufferedImage outputImage = new BufferedImage(targetWidth, targetHeight, BufferedImage.TYPE_INT_ARGB);
    //            outputImage.getGraphics().drawImage(resultingImage, 0, 0, null);
    //            return outputImage;
    //        }
    // Resize by centering the originalImage inside a larger fraem, without stretching or rescaling
    BufferedImage resizeCenterImage(Image originalImage, int targetWidth, int targetHeight) throws IOException {
        BufferedImage outputImage = new BufferedImage(targetWidth, targetHeight, BufferedImage.TYPE_INT_ARGB);
        int dx = (targetWidth - originalImage.getWidth(null)) / 2;
        int dy = (targetHeight - originalImage.getHeight(null)) / 2;
        outputImage.getGraphics().drawImage(originalImage, dx, dy, null);
        return outputImage;
    }

    @Override
    public List<Image> getResolutionVariants() {
        //            System.out.println("getResolutionVariants");
//        List<Image> lst = new LinkedList<>();
//        for (int h : srcHeights)
//            lst.add(new StubImage(h, h));
//        return lst;
        return images;
    }
    
//    private class StubImage extends Image {
//        int width, height;
//
//        public StubImage(int width, int height) {
//            this.width = width;
//            this.height = height;
//        }
//        
//        @Override
//        public int getWidth(ImageObserver observer) { 
//            System.out.println(this+".getWidth");
//            return width; 
//        }
//
//        @Override
//        public int getHeight(ImageObserver observer) { 
//            System.out.println(this+".getHeight");
//            return height; 
//        }
//
//        @Override
//        public ImageProducer getSource() { 
//            System.out.println(this+".getSource");
//            Image var = getResolutionVariant(width, height);
//            return var.getSource();
//        }
//
//        @Override
//        public Graphics getGraphics() { 
//            System.out.println(this+".getGraphics");
//            Image var = getResolutionVariant(width, height);
//            return var.getGraphics();
//        }
//
//        @Override
//        public Object getProperty(String string, ImageObserver io) { 
//            System.out.println(this+".getProperty");
//            Image var = getResolutionVariant(width, height);
//            return var.getProperty(string, io);
//        }
//    }
}
