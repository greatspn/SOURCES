/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package editor.gui;

import common.Util;
import editor.Main;
import java.util.HashMap;
import java.util.Map;
import javax.swing.ImageIcon;

/**
 *
 * @author elvio
 */
public class ResourceFactory {
    
    private static ResourceFactory instance;
    public static ResourceFactory getInstance() {
        if (instance == null)
            instance = new ResourceFactory();
        return instance;
    }
    
    public enum IconSize {
        SMALL_16_24_32(16, 24, 32),
        MEDIUM_24_32_48(24, 32, 48),
        LARGE_32_48_64(32, 48, 64);

        private IconSize(int size1, int size2, int size3) {
            this.size1 = size1;
            this.size2 = size2;
            this.size3 = size3;
        }
        final int size1, size2, size3;
    }
    
    public static ImageIcon loadImageIconOfSize(String imageName, IconSize size) {
        String load, test1, test2;
        switch (Main.getUiSize()) {
            case NORMAL:  
                load = imageName + size.size1+".png";
                test1 = imageName + size.size2 + ".png";
                test2 = imageName + size.size3 + ".png";
                break;
            case LARGE:
                test1 = imageName + size.size1+".png";
                load = imageName + size.size2 + ".png";
                test2 = imageName + size.size3 + ".png";
                break;
            case LARGER:
                test1 = imageName + size.size1+".png";
                test2 = imageName + size.size2 + ".png";
                load = imageName + size.size3 + ".png";
                break;
            default: throw new UnsupportedOperationException();
        }
        if (ResourceFactory.class.getResource("/editor/gui/icons/" + test1) == null) {
            System.out.println("WARNING: missing icon /editor/gui/icons/" + test1);
        }
        if (ResourceFactory.class.getResource("/editor/gui/icons/" + test2) == null) {
            System.out.println("WARNING: missing icon /editor/gui/icons/" + test2);
        }
        return Util.loadIcon("/editor/gui/icons/" + load);
    }
    
    private static final Map<String, ImageIcon> loadedResources16 = new HashMap<>();
    public static ImageIcon loadIcon16(String name) {
        if (!loadedResources16.containsKey(name)) {
            ImageIcon ico = loadImageIconOfSize(name, IconSize.SMALL_16_24_32);
            loadedResources16.put(name, ico);
            return ico;
        }
        return loadedResources16.get(name);
    }
    private static final Map<String, ImageIcon> loadedResources24 = new HashMap<>();
    public static ImageIcon loadIcon24(String name) {
        if (!loadedResources24.containsKey(name)) {
            ImageIcon ico = loadImageIconOfSize(name, IconSize.MEDIUM_24_32_48);
            loadedResources24.put(name, ico);
            return ico;
        }
        return loadedResources24.get(name);
    }
    private static final Map<String, ImageIcon> loadedResources32 = new HashMap<>();
    public static ImageIcon loadIcon32(String name) {
        if (!loadedResources32.containsKey(name)) {
            ImageIcon ico = loadImageIconOfSize(name, IconSize.LARGE_32_48_64);
            loadedResources32.put(name, ico);
            return ico;
        }
        return loadedResources32.get(name);
    }
    
        
//    public static void main(String[] args) {
//        String prefix = "src/editor/gui/icons";
//        String[] files = new File(prefix).list();
//        for (String f : files) {
////            System.out.println("file "+f);
//            if (f.length() < 6)
//                continue;
//            String radix6 = f.substring(0, f.length()-6);
//            String radix4 = f.substring(0, f.length()-4);
//            
//            if (f.endsWith("16.png") && 
//                new File(prefix+"/"+radix6+"24.png").exists() &&
//                new File(prefix+"/"+radix6+"32.png").exists()) {
////                System.out.println(radix6+" 16");
//            }
//            if (f.endsWith("24.png") && 
//                new File(prefix+"/"+radix6+"32.png").exists() &&
//                new File(prefix+"/"+radix6+"48.png").exists()) {
////                System.out.println(radix6+" 24");
//            }
//            if (f.endsWith("32.png") && 
//                new File(prefix+"/"+radix6+"48.png").exists() &&
//                new File(prefix+"/"+radix6+"64.png").exists()) {
////                System.out.println(radix6+" 32");
//            }
//            if (new File(prefix+"/"+radix4+"24.png").exists() &&
//                new File(prefix+"/"+radix4+"32.png").exists()) {
////                System.out.println(radix4+" 16");
//                System.out.println("rename "+radix4+" 16");
//            }
//            if (new File(prefix+"/"+radix4+"32.png").exists() &&
//                new File(prefix+"/"+radix4+"48.png").exists()) {
////                System.out.println(radix4+" 24");
//                System.out.println("rename "+radix4+" 24");
//            }
//            if (new File(prefix+"/"+radix4+"48.png").exists() &&
//                new File(prefix+"/"+radix4+"64.png").exists()) {
////                System.out.println(radix4+" 32");
//                System.out.println("rename "+radix4+" 32");
//            }
//        }
//    }
    
    ///////////////////////////////////////////////////////////////////////////

    public ImageIcon getAdvancedTools32() {
        return loadIcon32("advanced_tools");
    }
    
    public ImageIcon getArrowDown16() {
        return loadIcon16("arrow_down");
    }

    public ImageIcon getArrowUp16() {
        return loadIcon16("arrow_up");
    }

    public ImageIcon getBegin16() {
        return loadIcon16("begin");
    }

//    public ImageIcon getBegin24() {
//        return loadIcon24("begin");
//    }

    public ImageIcon getBegin32() {
        return loadIcon32("begin");
    }

    public ImageIcon getBot16() {
        return loadIcon16("bot");
    }
    
    public ImageIcon getBound32() {
        return loadIcon32("bound");
    }

    public ImageIcon getBroom32() {
        return loadIcon32("broom");
    }
    
    public ImageIcon getBuildCTMC32() {
        return loadIcon32("build_ctmc");
    }

    public ImageIcon getBuildRG32() {
        return loadIcon32("build_rg");
    }

    public ImageIcon getBuildSymRG32() {
        return loadIcon32("build_srg");
    }

//    public ImageIcon getBulletEdit16() {
//        return loadIcon16("bullet_edit");
//    }

    public ImageIcon getBulletToggleMinus16() {
        return loadIcon16("bullet_toggle_minus");
    }

    public ImageIcon getBulletTogglePlus16() {
        return loadIcon16("bullet_toggle_plus");
    }

    public ImageIcon getChangeBindings32() {
        return loadIcon32("change_bindings");
    }

    public ImageIcon getComment24() {
        return loadIcon24("comment");
    }

    public ImageIcon getConstantN24() {
        return loadIcon24("constantN");
    }

    public ImageIcon getConstantR24() {
        return loadIcon24("constantR");
    }

    public ImageIcon getCross16() {
        return loadIcon16("cross");
    }

//    public ImageIcon getCross24() {
//        return loadIcon24("cross");
//    }

    public ImageIcon getCross32() {
        return loadIcon32("cross");
    }

    public ImageIcon getCut16() {
        return loadIcon16("cut");
    }

//    public ImageIcon getCut24() {
//        return loadIcon24("cut");
//    }

    public ImageIcon getCut32() {
        return loadIcon32("cut");
    }

    public ImageIcon getDisk16() {
        return loadIcon16("disk");
    }

//    public ImageIcon getDisk24() {
//        return loadIcon24("disk");
//    }

    public ImageIcon getDisk32() {
        return loadIcon32("disk");
    }

    public ImageIcon getDiskEdit16() {
        return loadIcon16("disk_edit");
    }

//    public ImageIcon getDiskEdit24() {
//        return loadIcon24("disk_edit");
//    }

    public ImageIcon getDiskEdit32() {
        return loadIcon32("disk_edit");
    }

    public ImageIcon getDiskMultiple16() {
        return loadIcon16("disk_multiple");
    }

//    public ImageIcon getDiskMultiple24() {
//        return loadIcon24("disk_multiple");
//    }

    public ImageIcon getDiskMultiple32() {
        return loadIcon32("disk_multiple");
    }

    public ImageIcon getDistrib32() {
        return loadIcon32("distrib");
    }
    
    public ImageIcon getDropdown16() {
        return loadIcon16("dropdown");
    }

//    public ImageIcon getEamCenterOnly16() {
//        return loadIcon16("eam_center_only");
//    }
//
//    public ImageIcon getEamNseo16() {
//        return loadIcon16("eam_nseo");
//    }
//
//    public ImageIcon getEamNseoSquare16() {
//        return loadIcon16("eam_nseo_square");
//    }

//    public ImageIcon getEamSquare16() {
//        return loadIcon16("eam_square");
//    }

    public ImageIcon getEdgeBroken24() {
        return loadIcon24("edge_broken");
    }

    public ImageIcon getEdgePointAdd24() {
        return loadIcon24("edge_point_add");
    }

    public ImageIcon getEdgePointDelete24() {
        return loadIcon24("edge_point_delete");
    }
    
    public ImageIcon getEdgePointClear24() {
        return loadIcon24("edge_point_clear");
    }
    
    public ImageIcon getEmpty16() {
        return loadIcon16("empty");
    }

    public ImageIcon getError16() {
        return loadIcon16("error");
    }

    public ImageIcon getExclamation16() {
        return loadIcon16("exclamation");
    }
    
    public ImageIcon getExportExcel24() {
        return loadIcon24("export_excel");
    }
    
    public ImageIcon getFolder16() {
        return loadIcon16("folder");
    }
    
    public ImageIcon getRotation24() {
        return loadIcon24("rotation");
    }

    public ImageIcon getLast16() {
        return loadIcon16("last");
    }

//    public ImageIcon getLast24() {
//        return loadIcon24("last");
//    }

    public ImageIcon getLast32() {
        return loadIcon32("last");
    }

    public ImageIcon getLatexNode24() {
        return loadIcon24("latex_node");
    }

    public ImageIcon getLatexText24() {
        return loadIcon24("latex_text");
    }

    public ImageIcon getLock16() {
        return loadIcon16("lock");
    }

    public ImageIcon getLockOpen16() {
        return loadIcon16("lock_open");
    }
    
    public ImageIcon getMakeEditable32() {
        return loadIcon32("make_editable");
    }

    public ImageIcon getMeasure16() {
        return loadIcon16("measure");
    }

    public ImageIcon getMeasure24() {
        return loadIcon24("measure");
    }

    public ImageIcon getMeasureNew24() {
        return loadIcon24("measure_new");
    }

    public ImageIcon getMediaStopRed16() {
        return loadIcon16("media_stop_red");
    }

//    public ImageIcon getMediaStopRed24() {
//        return loadIcon24("media_stop_red");
//    }

    public ImageIcon getMediaStopRed32() {
        return loadIcon32("media_stop_red");
    }
    
//    public ImageIcon getMenuBasicOpen32() {
//        return loadIcon32("menu_basic_open");
//    }
//
//    public ImageIcon getMenuBasicClose32() {
//        return loadIcon32("menu_basic_close");
//    }

    public ImageIcon getMultiBindings32() {
        return loadIcon32("multi_bindings");
    }
    
    public ImageIcon getMultiNetAdd24() {
        return loadIcon24("multinet_add");
    }

    public ImageIcon getNewDta24() {
        return loadIcon24("new_dta");
    }

    public ImageIcon getNewMeasure24() {
        return loadIcon24("new_measure");
    }
    
    public ImageIcon getNewMultiNet24() {
        return loadIcon24("new_multinet");
    }
    
    public ImageIcon getNewAlgebraNet24() {
        return loadIcon24("new_algebranet");
    }
    
    public ImageIcon getNewUnfoldingNet24() {
        return loadIcon24("new_unfoldingnet");
    }
    
    public ImageIcon getNewNet24() {
        return loadIcon24("new_net");
    }

    public ImageIcon getNext16() {
        return loadIcon16("next");
    }

//    public ImageIcon getNext24() {
//        return loadIcon24("next");
//    }

    public ImageIcon getNext32() {
        return loadIcon32("next");
    }

    public ImageIcon getOk16() {
        return loadIcon16("ok");
    }

    public ImageIcon getOpen16() {
        return loadIcon16("open");
    }

//    public ImageIcon getOpen24() {
//        return loadIcon24("open");
//    }

    public ImageIcon getOpen32() {
        return loadIcon32("open");
    }
    
    public ImageIcon getOverlayOne16() {
        return loadIcon16("overlay_one");
    }

    public ImageIcon getOverlayPlus16() {
        return loadIcon16("overlay_plus");
    }

    public ImageIcon getPageDta16() {
        return loadIcon16("page_dta");
    }
    
    public ImageIcon getPageMultiNet16() {
        return loadIcon16("page_multinet");
    }
    
    public ImageIcon getPageAlgebra16() {
        return loadIcon16("page_algebra");
    }
    
    public ImageIcon getPageUnfolding16() {
        return loadIcon16("page_unfolding");
    }
    
    public ImageIcon getPageNet16() {
        return loadIcon16("page_net");
    }

    public ImageIcon getPageWhiteCopy16() {
        return loadIcon16("page_white_copy");
    }

//    public ImageIcon getPageWhiteCopy24() {
//        return loadIcon24("page_white_copy");
//    }

    public ImageIcon getPageWhiteCopy32() {
        return loadIcon32("page_white_copy");
    }

    public ImageIcon getPageWhiteNew16() {
        return loadIcon16("page_white_new");
    }
    public ImageIcon getPageWhiteNew32() {
        return loadIcon32("page_white_new");
    }

    public ImageIcon getPageWhitePaste16() {
        return loadIcon16("page_white_paste");
    }

//    public ImageIcon getPageWhitePaste24() {
//        return loadIcon24("page_white_paste");
//    }

    public ImageIcon getPageWhitePaste32() {
        return loadIcon32("page_white_paste");
    }

    public ImageIcon getPinv32() {
        return loadIcon32("pinv");
    }

    public ImageIcon getPlay16() {
        return loadIcon16("play");
    }

//    public ImageIcon getPlay24() {
//        return loadIcon24("play");
//    }

    public ImageIcon getPlay32() {
        return loadIcon32("play");
    }

    public ImageIcon getPnproNew16() {
        return loadIcon16("pnpro_new");
    }

    public ImageIcon getPnproNew32() {
        return loadIcon32("pnpro_new");
    }

    public ImageIcon getPnproj32() {
        return loadIcon32("pnproj");
    }

    public ImageIcon getPnproj16() {
        return loadIcon16("pnproj");
    }

    public ImageIcon getPrevious16() {
        return loadIcon16("previous");
    }

//    public ImageIcon getPrevious24() {
//        return loadIcon24("previous");
//    }

    public ImageIcon getPrevious32() {
        return loadIcon32("previous");
    }

    public ImageIcon getPrinter16() {
        return loadIcon16("printer");
    }

    public ImageIcon getRedo16() {
        return loadIcon16("redo");
    }

//    public ImageIcon getRedo24() {
//        return loadIcon24("redo");
//    }

    public ImageIcon getRedo32() {
        return loadIcon32("redo");
    }

    public ImageIcon getRename16() {
        return loadIcon16("rename");
    }

    public ImageIcon getResizeHoriz24() {
        return loadIcon24("resize_horiz");
    }

    public ImageIcon getResizeVert24() {
        return loadIcon24("resize_vert");
    }

    public ImageIcon getRuler24() {
        return loadIcon24("ruler");
    }

    public ImageIcon getSnapToGrid24() {
        return loadIcon24("snap_to_grid");
    }
    
    public ImageIcon getSum32() {
        return loadIcon32("sum");
    }

    public ImageIcon getTextAlignCenter16() {
        return loadIcon16("text_align_center");
    }

    public ImageIcon getTextAlignJustity16() {
        return loadIcon16("text_align_justity");
    }

    public ImageIcon getTextAlignLeft16() {
        return loadIcon16("text_align_left");
    }

    public ImageIcon getTextAlignRight16() {
        return loadIcon16("text_align_right");
    }

    public ImageIcon getTextBold16() {
        return loadIcon16("text_bold");
    }

    public ImageIcon getTextItalic16() {
        return loadIcon16("text_italic");
    }

    public ImageIcon getTextVertBottom16() {
        return loadIcon16("text_vert_bottom");
    }

    public ImageIcon getTextVertCenter16() {
        return loadIcon16("text_vert_center");
    }

    public ImageIcon getTextVertTop16() {
        return loadIcon16("text_vert_top");
    }
    
    public ImageIcon getThreeTags24() {
        return loadIcon24("three_tags");
    }

    public ImageIcon getTime16() {
        return loadIcon16("time");
    }

    public ImageIcon getTimeDelete32() {
        return loadIcon32("time_delete");
    }

    public ImageIcon getTimeElapse16() {
        return loadIcon16("time_elapse");
    }

    public ImageIcon getTimeGo32() {
        return loadIcon32("time_go");
    }

    public ImageIcon getTinv32() {
        return loadIcon32("tinv");
    }
    
    public ImageIcon getCheckBoxSelected16() {
        return loadIcon16(Util.isOSX() ? "checkbox_selected_osx" : "checkbox_selected");
    }
    public ImageIcon getCheckBoxDeselected16() {
        return loadIcon16(Util.isOSX() ? "checkbox_deselected_osx" : "checkbox_deselected");
    }
    public ImageIcon getCheckBoxIndeterminate16() {
        return loadIcon16(Util.isOSX() ? "checkbox_indeterminate_osx" : "checkbox_indeterminate");
    }

    public ImageIcon getToolAddClockvar24() {
        return loadIcon24("tool_add_clockvar");
    }

    public ImageIcon getToolAddLoc24() {
        return loadIcon24("tool_add_loc");
    }

    public ImageIcon getToolArrow24() {
        return loadIcon24("tool_arrow");
    }

    public ImageIcon getToolArrowImmed24() {
        return loadIcon24("tool_arrow_immed");
    }
    public ImageIcon getToolArrowImmed16() {
        return loadIcon16("tool_arrow_immed");
    }

    public ImageIcon getToolArrowTriangle16() {
        return loadIcon16("tool_arrow_triangle");
    }
    public ImageIcon getToolArrowTriangle24() {
        return loadIcon24("tool_arrow_triangle");
    }

    public ImageIcon getToolColor24() {
        return loadIcon24("tool_color");
    }

    public ImageIcon getToolColorvar24() {
        return loadIcon24("tool_colorvar");
    }

    public ImageIcon getToolCplace24() {
        return loadIcon24("tool_cplace");
    }

    public ImageIcon getToolDetTrn16() {
        return loadIcon16("tool_det_trn");
    }
    public ImageIcon getToolDetTrn24() {
        return loadIcon24("tool_det_trn");
    }

    public ImageIcon getToolFlowtrn16() {
        return loadIcon16("tool_flowtrn");
    }
    public ImageIcon getToolFlowtrn24() {
        return loadIcon24("tool_flowtrn");
    }

    public ImageIcon getToolImmTrn16() {
        return loadIcon16("tool_imm_trn");
    }
    public ImageIcon getToolImmTrn24() {
        return loadIcon24("tool_imm_trn");
    }

    public ImageIcon getToolInhib24() {
        return loadIcon24("tool_inhib");
    }

    public ImageIcon getToolInitLoc24() {
        return loadIcon24("tool_init_loc");
    }

    public ImageIcon getToolLocBot24() {
        return loadIcon24("tool_loc_bot");
    }

    public ImageIcon getToolLocTop24() {
        return loadIcon24("tool_loc_top");
    }

    public ImageIcon getToolPlace24() {
        return loadIcon24("tool_place");
    }

    public ImageIcon getToolSelect24() {
        return loadIcon24("tool_select");
    }

    public ImageIcon getToolTemplateAct24() {
        return loadIcon24("tool_template_act");
    }

    public ImageIcon getToolTemplateClr24() {
        return loadIcon24("tool_template_clr");
    }

    public ImageIcon getToolTemplateN24() {
        return loadIcon24("tool_template_N");
    }

    public ImageIcon getToolTemplateR24() {
        return loadIcon24("tool_template_R");
    }

    public ImageIcon getToolTemplateSP24() {
        return loadIcon24("tool_template_SP");
    }

    public ImageIcon getToolTrn16() {
        return loadIcon16("tool_trn");
    }
    public ImageIcon getToolTrn24() {
        return loadIcon24("tool_trn");
    }

    public ImageIcon getTop16() {
        return loadIcon16("top");
    }

    public ImageIcon getUndo16() {
        return loadIcon16("undo");
    }

//    public ImageIcon getUndo24() {
//        return loadIcon24("undo");
//    }

    public ImageIcon getUndo32() {
        return loadIcon32("undo");
    }
    
    public ImageIcon getUnfolding32() {
        return loadIcon32("unfolding");
    }

    public ImageIcon getZoomIn16() {
        return loadIcon16("zoom_in");
    }

    public ImageIcon getZoomOut16() {
        return loadIcon16("zoom_out");
    }

    public ImageIcon getZoomSame16() {
        return loadIcon16("zoom_same");
    }
}
