/*
 * Swing Version
 */

import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;

public class MultiSolve extends JFrame{
    
    static final long serialVersionUID=1L;
    CheckBoxListener myListener = new CheckBoxListener();
    Vector gnucurves = new Vector();
    JList gnuList = new JList();

    final JTextArea gnufileTextArea = new JTextArea(8,40);
    
    final JCheckBox timeCheckBox = new JCheckBox("Transient analysis", true);
    final JTextField xaxisTextField = new JTextField(10);
    final JTextField xminTextField = new JTextField(5);
    final JCheckBox xminCheckBox = new JCheckBox("Auto", true);
    final JTextField xmaxTextField = new JTextField(5);
    final JCheckBox xmaxCheckBox = new JCheckBox("Auto", true);
    final JTextField yminTextField = new JTextField(5);
    final JCheckBox yminCheckBox = new JCheckBox("Auto", true);
    final JTextField ymaxTextField = new JTextField(5);
    final JCheckBox ymaxCheckBox = new JCheckBox("Auto", true);
    
    final JRadioButton gspnRadioButton = new JRadioButton("GSPN", true);
    final JRadioButton swnRadioButton = new JRadioButton("SWN", false);
    final JRadioButton simRadioButton = new JRadioButton("Simulation", false);
    final JRadioButton exactRadioButton = new JRadioButton("Analytic", true);
    final JRadioButton ordRadioButton = new JRadioButton("Ordinary", true);
    final JRadioButton symRadioButton = new JRadioButton("Symmetric", false);
    
    final JRadioButton leftRadioButton = new JRadioButton("Left", false);
    final JRadioButton rightRadioButton = new JRadioButton("Right", true);
    final JRadioButton topRadioButton = new JRadioButton("Top", true);
    final JRadioButton bottomRadioButton = new JRadioButton("Bottom", false);
    
    final JRadioButton linesRadioButton = new JRadioButton("Lines", true);
    final JRadioButton pointsRadioButton = new JRadioButton("Points", false);
    final JRadioButton linespointsRadioButton = new JRadioButton("Lines and points", false);
    
    final JTextField initTransTextField = new JTextField(10);
    final JTextField batchSpTextField = new JTextField(10);
    final JTextField minLengthTextField = new JTextField(10);
    final JTextField maxLengthTextField = new JTextField(10);
    final JTextField seedTextField = new JTextField(10);
    final JTextField accuracyTextField = new JTextField(10);
    String[] confStrings = { "60", "70", "80", "90", "95", "99" };
    JComboBox confList = new JComboBox(confStrings);
    
    // ---------------
    // Execute command
	private int execProcess(String commandLine)
    {
	try
	    {
		System.out.println("trying to exec: " + commandLine);
		Process p = Runtime.getRuntime().exec(commandLine);
		
/*		BufferedReader b = new BufferedReader(new InputStreamReader(p.getInputStream()));
		String l;
		while ((l = b.readLine()) != null) 
		    {
			Debug.message("server.exec",l);
		    }
*/		
		int ExitVal = p.waitFor();  
		System.out.println("terminate with: " + ExitVal);
		return ExitVal;
	    } 
	catch (Exception e)
	    {
		System.out.println("Error: " + e.getMessage());
		return ERROR;
	    }
    } // of execProcess

    private int StringCheck(String str)
    {
	int i1,ok=1;
	
	for(i1=0;i1<str.length();i1++)
	    if(str.charAt(i1)!=' ' && 
	       str.charAt(i1)!='.' && 
	       str.charAt(i1)!='0' &&
	       str.charAt(i1)!='1' &&
	       str.charAt(i1)!='2' &&
	       str.charAt(i1)!='3' &&
	       str.charAt(i1)!='4' &&
	       str.charAt(i1)!='5' &&
	       str.charAt(i1)!='6' &&
	       str.charAt(i1)!='7' &&
	       str.charAt(i1)!='8' &&
	       str.charAt(i1)!='9' ) ok=0;
	
	return ok;
    }

    private int ParamCount(String str)
    {
	int i1,c=0,inc=1;
	
	for(i1=0;i1<str.length();i1++)
	    {
		if(str.charAt(i1)==' ') inc=1;
		if(str.charAt(i1)!=' ' && inc==1) 
		    {
			c++;
			inc=0;
		    }
	    }
	return c;
    }

    public MultiSolve(String netdir, String gh) {
	super("MultiSolve");

	final String greathome = new String(gh);
        GridBagLayout gridBag = new GridBagLayout();
        Container contentPane = getContentPane();
        contentPane.setLayout(gridBag);
        GridBagConstraints c = new GridBagConstraints();

	// creating and positioning file chooser's button and nets name label
	final JFileChooser netChooser = new JFileChooser(netdir);
	netChooser.addChoosableFileFilter(new NetFilter());
	JButton netButton = new JButton("Choose a net:");
	final JTextField netTextField = new JTextField(30);
	final JTextField gnufileTextField = new JTextField(20);
	netButton.addActionListener(new ActionListener() 
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    int returnVal = netChooser.showOpenDialog(MultiSolve.this);
		    
		    if (returnVal == JFileChooser.APPROVE_OPTION) 
			{
			    String netname;
			    File file = netChooser.getSelectedFile();
			    netname = file.getPath();
			    netname = netname.substring(0,netname.lastIndexOf(".net"));
			    netTextField.setText(netname);
			    gnufileTextField.setText(netname);
			} 
		}
	    });
	JPanel netPanel = new JPanel();
	netPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	netPanel.setBorder(BorderFactory.createCompoundBorder
				 (BorderFactory.createEtchedBorder(),
				  netPanel.getBorder()));
        GridBagLayout netGridBag = new GridBagLayout();
        netPanel.setLayout(netGridBag);
	c.gridx=0;
	c.gridy=0;
	netGridBag.setConstraints(netButton,c);
	netPanel.add(netButton);
	c.weightx=0.2;
	c.gridx=1;
	c.gridy=0;
	c.fill=GridBagConstraints.HORIZONTAL;
	netGridBag.setConstraints(netTextField,c);
	netPanel.add(netTextField);
	c.weightx=0;	

	c.gridx=0;
	c.gridy=0;
	c.gridwidth=3;
	gridBag.setConstraints(netPanel,c);
	contentPane.add(netPanel);
	c.gridwidth=1;

	// Creating and positioning parameter panel
	JPanel parameterPanel = new JPanel();
	parameterPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	parameterPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     parameterPanel.getBorder()));
        GridBagLayout parameterGridBag = new GridBagLayout();
        parameterPanel.setLayout(parameterGridBag);
	JLabel ptitleLabel = new JLabel("Parameters of individual curves",JLabel.LEFT);
	ptitleLabel.setForeground(new Color(0,0,0));
	JLabel ptitleLabel2 = new JLabel("(these may be empty)",JLabel.LEFT);
	JLabel parameterLabel = new JLabel("Names:",JLabel.RIGHT);
	JLabel plowerLabel = new JLabel("Lower limits:",JLabel.RIGHT);
	JLabel pupperLabel = new JLabel("Upper limits:",JLabel.RIGHT);
	JLabel pstepLabel = new JLabel("Stepsizes:",JLabel.RIGHT);
	final JTextField parameterTextField = new JTextField(10);
	final JTextField plowerTextField = new JTextField(10);
	final JTextField pupperTextField = new JTextField(10);
	final JTextField pstepTextField = new JTextField(10);

	c.fill=GridBagConstraints.BOTH;
	c.gridx=0;
	c.gridy=1;
	c.weightx=1;
	gridBag.setConstraints(parameterPanel,c);
	contentPane.add(parameterPanel);
	c.weightx=0;
	c.gridx=0;
	c.gridy=0;
	c.gridwidth=2;
	c.insets = new Insets(0,0,0,0);
	parameterGridBag.setConstraints(ptitleLabel,c);
	parameterPanel.add(ptitleLabel);
	c.weightx=0;
	c.gridx=0;
	c.gridy=1;
	c.gridwidth=2;
	c.insets = new Insets(0,0,5,0);
	parameterGridBag.setConstraints(ptitleLabel2,c);
	parameterPanel.add(ptitleLabel2);
	c.insets = new Insets(0,0,0,0);
	c.gridwidth=1;
	c.gridx=0;
	c.gridy=2;
	parameterGridBag.setConstraints(parameterLabel,c);
	parameterPanel.add(parameterLabel);
	c.gridx=0;
	c.gridy=3;
	parameterGridBag.setConstraints(plowerLabel,c);
	parameterPanel.add(plowerLabel);
	c.gridx=0;
	c.gridy=4;
	parameterGridBag.setConstraints(pupperLabel,c);
	parameterPanel.add(pupperLabel);
	c.gridx=0;
	c.gridy=5;
	parameterGridBag.setConstraints(pstepLabel,c);
	parameterPanel.add(pstepLabel);
	c.gridx=1;
	c.gridy=2;
	parameterGridBag.setConstraints(parameterTextField,c);
	parameterPanel.add(parameterTextField);
	c.gridx=1;
	c.gridy=3;
	parameterGridBag.setConstraints(plowerTextField,c);
	parameterPanel.add(plowerTextField);
	c.gridx=1;
	c.gridy=4;
	parameterGridBag.setConstraints(pupperTextField,c);
	parameterPanel.add(pupperTextField);
	c.gridx=1;
	c.gridy=5;
	parameterGridBag.setConstraints(pstepTextField,c);
	parameterPanel.add(pstepTextField);

	// Creating and positioning Time panel
	JPanel timePanel = new JPanel();
	timePanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	timePanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     timePanel.getBorder()));
        GridBagLayout timeGridBag = new GridBagLayout();
        timePanel.setLayout(timeGridBag);
	JLabel ttitleLabel = new JLabel("Parameters of x-axis",JLabel.LEFT);
	ttitleLabel.setForeground(new Color(0,0,0));
	ttitleLabel.setVerticalTextPosition(JLabel.TOP);
	JLabel xaxisLabel = new JLabel("Variable:",JLabel.RIGHT);
	xaxisTextField.setText("Time");
	JLabel tlowerLabel = new JLabel("Lower limit:",JLabel.RIGHT);
	JLabel tupperLabel = new JLabel("Upper limit:",JLabel.RIGHT);
	JLabel tstepLabel = new JLabel("Stepsize:",JLabel.RIGHT);
	xaxisTextField.setBackground(new Color(230,230,230));
	xaxisTextField.setEnabled(false);
	timeCheckBox.setHorizontalTextPosition(SwingConstants.LEFT);
	timeCheckBox.addItemListener(myListener);
	timeCheckBox.setForeground(tlowerLabel.getForeground());
	final JTextField tlowerTextField = new JTextField(10);
	final JTextField tupperTextField = new JTextField(10);
	final JTextField tstepTextField = new JTextField(10);

	c.gridx=1;
	c.gridy=1;
	c.weightx=1;
	gridBag.setConstraints(timePanel,c);
	contentPane.add(timePanel);
	c.weightx=0;
	c.gridx=0;
	c.gridy=0;
	c.gridwidth=2;
	c.insets = new Insets(0,0,5,0);
	timeGridBag.setConstraints(ttitleLabel,c);
	timePanel.add(ttitleLabel);
	c.insets = new Insets(0,0,0,0);
	c.gridwidth=2;
	c.gridx=0;
	c.gridy=1;
	timeGridBag.setConstraints(timeCheckBox,c);
	timePanel.add(timeCheckBox);
	c.gridwidth=1;
	c.gridx=0;
	c.gridy=2;
	timeGridBag.setConstraints(xaxisLabel,c);
	timePanel.add(xaxisLabel);
	c.gridx=1;
	c.gridy=2;
	timeGridBag.setConstraints(xaxisTextField,c);
	timePanel.add(xaxisTextField);
	c.gridx=0;
	c.gridy=3;
	timeGridBag.setConstraints(tlowerLabel,c);
	timePanel.add(tlowerLabel);
	c.gridx=0;
	c.gridy=4;
	timeGridBag.setConstraints(tupperLabel,c);
	timePanel.add(tupperLabel);
	c.gridx=0;
	c.gridy=5;
	timeGridBag.setConstraints(tstepLabel,c);
	timePanel.add(tstepLabel);
	c.gridx=1;
	c.gridy=3;
	timeGridBag.setConstraints(tlowerTextField,c);
	timePanel.add(tlowerTextField);
	c.gridx=1;
	c.gridy=4;
	timeGridBag.setConstraints(tupperTextField,c);
	timePanel.add(tupperTextField);
	c.gridx=1;
	c.gridy=5;
	timeGridBag.setConstraints(tstepTextField,c);
	timePanel.add(tstepTextField);

	// Creating and positioning simulation panel
	JPanel simulationPanel = new JPanel();
	simulationPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	simulationPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     simulationPanel.getBorder()));
        GridBagLayout simulationGridBag = new GridBagLayout();
        simulationPanel.setLayout(simulationGridBag);
	JLabel stitleLabel = new JLabel("Parameters of simulation",JLabel.LEFT);
	stitleLabel.setForeground(new Color(0,0,0));
      	JLabel initTransLabel = new JLabel("Initial transitory:",JLabel.RIGHT);
	JLabel batchSpLabel = new JLabel("Batch spacing:",JLabel.RIGHT);
	JLabel minLengthLabel = new JLabel("Min. batch length:",JLabel.RIGHT);
	JLabel maxLengthLabel = new JLabel("Max. batch length:",JLabel.RIGHT);
	JLabel seedLabel = new JLabel("Seed:",JLabel.RIGHT);
	JLabel accuracyLabel = new JLabel("Accuracy:",JLabel.RIGHT);
	JLabel confLabel = new JLabel("Conf. level:",JLabel.RIGHT); 
	exactRadioButton.addItemListener(myListener);
	initTransTextField.setText("1000");
	batchSpTextField.setText("1000");
	minLengthTextField.setText("1000");
	maxLengthTextField.setText("2000");
	seedTextField.setText("1");
	accuracyTextField.setText("15");
	initTransTextField.setBackground(new Color(230,230,230));
	initTransTextField.setEnabled(false);
	batchSpTextField.setBackground(new Color(230,230,230));
	batchSpTextField.setEnabled(false);
	minLengthTextField.setBackground(new Color(230,230,230));
	minLengthTextField.setEnabled(false);
	maxLengthTextField.setBackground(new Color(230,230,230));
	maxLengthTextField.setEnabled(false);
	seedTextField.setBackground(new Color(230,230,230));
	seedTextField.setEnabled(false);
	accuracyTextField.setBackground(new Color(230,230,230));
	accuracyTextField.setEnabled(false);
	confList.setEnabled(false);

	c.fill=GridBagConstraints.BOTH;
	c.gridx=2;
	c.gridy=1;
	c.weightx=1;
	gridBag.setConstraints(simulationPanel,c);
	contentPane.add(simulationPanel);
	c.weightx=0;
	c.gridx=0;
	c.gridy=0;
	c.gridwidth=2;
	c.insets = new Insets(0,0,5,0);
	simulationGridBag.setConstraints(stitleLabel,c);
	simulationPanel.add(stitleLabel);
	c.insets = new Insets(0,0,0,0);
	c.gridwidth=1;
	c.gridx=0;
	c.gridy=1;
	simulationGridBag.setConstraints(initTransLabel,c);
	simulationPanel.add(initTransLabel);
	c.gridx=0;
	c.gridy=2;
	simulationGridBag.setConstraints(batchSpLabel,c);
	simulationPanel.add(batchSpLabel);
	c.gridx=0;
	c.gridy=3;
	simulationGridBag.setConstraints(minLengthLabel,c);
	simulationPanel.add(minLengthLabel);
	c.gridx=0;
	c.gridy=4;
	simulationGridBag.setConstraints(maxLengthLabel,c);
	simulationPanel.add(maxLengthLabel);
	c.gridx=0;
	c.gridy=5;
	simulationGridBag.setConstraints(seedLabel,c);
	simulationPanel.add(seedLabel);
	c.gridx=0;
	c.gridy=6;
	simulationGridBag.setConstraints(accuracyLabel,c);
	simulationPanel.add(accuracyLabel);
	c.gridx=0;
	c.gridy=7;
	simulationGridBag.setConstraints(confLabel,c);
	simulationPanel.add(confLabel);
	c.gridx=1;
	c.gridy=1;
	simulationGridBag.setConstraints(initTransTextField,c);
	simulationPanel.add(initTransTextField);
	c.gridx=1;
	c.gridy=2;
	simulationGridBag.setConstraints(batchSpTextField,c);
	simulationPanel.add(batchSpTextField);
	c.gridx=1;
	c.gridy=3;
	simulationGridBag.setConstraints(minLengthTextField,c);
	simulationPanel.add(minLengthTextField);
	c.gridx=1;
	c.gridy=4;
	simulationGridBag.setConstraints(maxLengthTextField,c);
	simulationPanel.add(maxLengthTextField);
	c.gridx=1;
	c.gridy=5;
	simulationGridBag.setConstraints(seedTextField,c);
	simulationPanel.add(seedTextField);
	c.gridx=1;
	c.gridy=6;
	simulationGridBag.setConstraints(accuracyTextField,c);
	simulationPanel.add(accuracyTextField);
	c.gridx=1;
	c.gridy=7;
	simulationGridBag.setConstraints(confList,c);
	simulationPanel.add(confList);


	// creating results field and calc button
	JPanel calcPanel = new JPanel();
	JLabel netTypeLabel = new JLabel("Type of net:");
	netTypeLabel.setForeground(new Color(0,0,0));
	JLabel swnTypeLabel = new JLabel("Type of SWN:");
	swnTypeLabel.setForeground(new Color(0,0,0));
	JLabel calcTypeLabel = new JLabel("Calculation:");
	calcTypeLabel.setForeground(new Color(0,0,0));
	ButtonGroup gspn_swnGroup = new ButtonGroup();
	ButtonGroup sim_exactGroup = new ButtonGroup();
	ButtonGroup ord_symGroup = new ButtonGroup();
	gspn_swnGroup.add(gspnRadioButton);	
	gspnRadioButton.addItemListener(myListener);
	gspn_swnGroup.add(swnRadioButton);
	sim_exactGroup.add(exactRadioButton);	
	sim_exactGroup.add(simRadioButton);
	exactRadioButton.setEnabled(false);	
	simRadioButton.setEnabled(false);
	ord_symGroup.add(ordRadioButton);	
	ord_symGroup.add(symRadioButton);
	ordRadioButton.setEnabled(false);	
	symRadioButton.setEnabled(false);

	final JTextField resultTextField = new JTextField(10);
	JLabel resultLabel = new JLabel("Results to calculate:");
	resultLabel.setForeground(new Color(0,0,0));
	calcPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	calcPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     calcPanel.getBorder()));
        GridBagLayout calcGridBag = new GridBagLayout();
        calcPanel.setLayout(calcGridBag);
	JButton calcButton = new JButton("Perform calculations");
	final JOptionPane frame = new JOptionPane();
	calcButton.addActionListener(new ActionListener() 
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    int ok=1,l1,l2,l3,l4;

		    // Values are checked here
		    if(netTextField.getText().length()==0)
			{
			    JOptionPane.showMessageDialog(frame, "No net given.");
			    ok=0;
			}
		    File tocheck = new File ( netTextField.getText()+".net" );
		    File tocheck2 = new File ( netTextField.getText()+".def" );
		    if( (!tocheck.exists() || !tocheck2.exists() ) && ok!=0)
			{
			    JOptionPane.showMessageDialog(frame, "Net doesn't exist.");
			    ok=0;
			}
		    l1=parameterTextField.getText().length();
		    l2=plowerTextField.getText().length();
		    l3=pupperTextField.getText().length();
		    l4=pstepTextField.getText().length();
		    if( ok!=0 && !( l1==0 && l2==0 && l3==0 && l4==0 || l1!=0 && l2!=0 && l3!=0 && l4!=0) )
			{
			    JOptionPane.showMessageDialog(frame, 
			      "Fields of curve-parameters may be either all empty or all non-empty.");
			    ok=0;
			}
		    l1=ParamCount(parameterTextField.getText());
		    if( ok!=0 && (l1!=ParamCount(plowerTextField.getText()) ||
				  l1!=ParamCount(pupperTextField.getText()) ||
				  l1!=ParamCount(pstepTextField.getText())))
			{
			    JOptionPane.showMessageDialog(frame, 
			       "Fields of curve-parameters have different number of entries.");
			    ok=0;
			}
		    if( ok!=0 && (StringCheck(plowerTextField.getText())==0 ||
				  StringCheck(pupperTextField.getText())==0 ||
				  StringCheck(pstepTextField.getText())==0 ))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "Curve-limits or stepsizes contain non-numeric characters.");
			    ok=0;
			}
		    l1=xaxisTextField.getText().length();
		    l2=tlowerTextField.getText().length();
		    l3=tupperTextField.getText().length();
		    l4=tstepTextField.getText().length();
		    if( ok!=0 && !timeCheckBox.isSelected() && (l1==0||l2==0||l3==0||l4==0))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "One or more fields describing the x-axis are empty.");
			    ok=0;
			}
		    if( ok!=0 && timeCheckBox.isSelected() && (l2==0||l3==0||l4==0))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "One or more fields describing the x-axis are empty.");
			    ok=0;
			}
		    if( ok!=0 && ( ParamCount(xaxisTextField.getText())>1 ||
				  ParamCount(tlowerTextField.getText())>1 ||
				  ParamCount(tupperTextField.getText())>1 || 
				  ParamCount(tstepTextField.getText())>1 ))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "Descriptors of x-axis may have only one parameter.");
			    ok=0;
			}
		    if( ok!=0 && (StringCheck(tlowerTextField.getText())==0 ||
				  StringCheck(tupperTextField.getText())==0 ||
				  StringCheck(tstepTextField.getText())==0 ))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "X-axis-limits or stepsizes contain\n non-numeric characters.");
			    ok=0;
			}
		    if( ok!=0 && simRadioButton.isSelected() && 
			(initTransTextField.getText().length()==0 ||
			 batchSpTextField.getText().length()==0 ||
			 minLengthTextField.getText().length()==0 ||
			 maxLengthTextField.getText().length()==0 ||
			 accuracyTextField.getText().length()==0 ||
			 seedTextField.getText().length()==0 ))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "One or more fields describing simulation are empty");
			    ok=0;
			}
		    if( ok!=0 && (StringCheck(initTransTextField.getText())==0 ||
				  StringCheck(batchSpTextField.getText())==0 ||
				  StringCheck(minLengthTextField.getText())==0 ||
				  StringCheck(maxLengthTextField.getText())==0 ||
				  StringCheck(accuracyTextField.getText())==0 ||
				  StringCheck(seedTextField.getText())==0 ))
			{
			    JOptionPane.showMessageDialog(frame, 
			      "Simulation-parameters contain non-numeric characters.");
			    ok=0;
			}
		    if( ok!=0 && resultTextField.getText().length()==0 )
			{
			    JOptionPane.showMessageDialog(frame, 
			      "Measures to compute are not given.");
			    ok=0;
			}
	
		    if(ok==1)
			{
			    // write parameters to files
			    try 
				{
				    if( !timeCheckBox.isSelected() )
					{
					    FileOutputStream variableFile = 
						new FileOutputStream(netTextField.getText()+"_egyeb.variable");
					    PrintWriter variableWriter = 
						new PrintWriter(variableFile);
					    variableWriter.print(xaxisTextField.getText());
					    variableWriter.close();
					}
				    
				    if( simRadioButton.isSelected() )
					{
					    FileOutputStream simulationFile = 
						new FileOutputStream(netTextField.getText()+"_egyeb.simulation");
					    PrintWriter simulationWriter = 
						new PrintWriter(simulationFile);
					    simulationWriter.print(" -f " + initTransTextField.getText() +
								   " -t " + batchSpTextField.getText() +
								   " -m " + minLengthTextField.getText() +
								   " -M " + maxLengthTextField.getText() +
								   " -a " + accuracyTextField.getText() +
								   " -s " + seedTextField.getText() +
								   " -c " + confList.getSelectedItem());
					    simulationWriter.close();
					}
				    
				    FileOutputStream measuresFile = 
					new FileOutputStream(netTextField.getText()+"_egyeb.measures");
				    PrintWriter measuresWriter = 
					new PrintWriter(measuresFile);
				    measuresWriter.print(resultTextField.getText());
				    measuresWriter.close();
				    
				    FileOutputStream parametersFile = 
					new FileOutputStream(netTextField.getText()+"_egyeb.parameters");
				    PrintWriter parametersWriter = 
					new PrintWriter(parametersFile);
				    parametersWriter.print(parameterTextField.getText());
				    parametersWriter.close();
				    
				    FileOutputStream lboundsFile = 
					new FileOutputStream(netTextField.getText()+"_egyeb.lbounds");
				    PrintWriter lboundsWriter = 
					new PrintWriter(lboundsFile);
				    lboundsWriter.print(plowerTextField.getText());
				    lboundsWriter.close();
				    
				    FileOutputStream uboundsFile = 
					new FileOutputStream(netTextField.getText()+"_egyeb.ubounds");
				    PrintWriter uboundsWriter = 
					new PrintWriter(uboundsFile);
				    uboundsWriter.print(pupperTextField.getText());
				    uboundsWriter.close();
				    
				    FileOutputStream stepsFile = 
					new FileOutputStream(netTextField.getText()+"_egyeb.steps");
				    PrintWriter stepsWriter = 
					new PrintWriter(stepsFile);
				    stepsWriter.print(pstepTextField.getText());
				    stepsWriter.close();
				}
			    catch (FileNotFoundException f) {}
			    
			    // input parameters should be checked
			    if( swnRadioButton.isSelected() && timeCheckBox.isSelected() && 
				exactRadioButton.isSelected() && symRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_tr_ex_sym " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    if( swnRadioButton.isSelected() && !timeCheckBox.isSelected() &&
				exactRadioButton.isSelected() && symRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_st_ex_sym " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    
			    if( swnRadioButton.isSelected() && timeCheckBox.isSelected() && 
				exactRadioButton.isSelected() && ordRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_tr_ex_ord " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    if( swnRadioButton.isSelected() && !timeCheckBox.isSelected() &&
				exactRadioButton.isSelected() && ordRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_st_ex_ord " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    
			    if( swnRadioButton.isSelected() && !timeCheckBox.isSelected() &&
				simRadioButton.isSelected() && ordRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_st_sim_ord " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    if( swnRadioButton.isSelected() && !timeCheckBox.isSelected() &&
				simRadioButton.isSelected() && symRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_st_sim_sym " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    
			    if( gspnRadioButton.isSelected() && timeCheckBox.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/gspn_tr_ex " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    if( gspnRadioButton.isSelected() && !timeCheckBox.isSelected() &&
				exactRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/gspn_st_ex " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}
			    if( gspnRadioButton.isSelected() && !timeCheckBox.isSelected() &&
				simRadioButton.isSelected() )
				{
				    execProcess("xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/swn_st_sim_ord " + netTextField.getText() + " " +
						tlowerTextField.getText() + " " +
						tupperTextField.getText() + " " +
						tstepTextField.getText());
				}

			    File tocheck3 = new File ( netTextField.getText()+".net.errors" );
			    if( tocheck3.exists() )
				{
				    gnucurves.clear();
				    gnuList.setListData(gnucurves);
				    gnufileTextArea.setText("");
				    try {
					FileInputStream errorFIS = new FileInputStream(netTextField.getText() + ".net.errors");
					BufferedReader errorBR = new BufferedReader(new InputStreamReader(errorFIS));
					while(errorBR.ready())
					    JOptionPane.showMessageDialog(frame, errorBR.readLine());
					errorFIS.close();
				    }
				    catch(IOException f) {}
				    tocheck3.delete();
				    File tocheck4 = new File ( netTextField.getText()+".net.curves" );
				    if( tocheck4.exists() ) tocheck4.delete();
				}
			    else
				{
				    gnucurves.clear();
				    gnuList.setListData(gnucurves);
				    try {
					FileInputStream curvesFIS = new FileInputStream(netTextField.getText() + ".net.curves");
					BufferedReader curvesBR = new BufferedReader(new InputStreamReader(curvesFIS));
					while(curvesBR.ready())
					    gnucurves.add(curvesBR.readLine());
					curvesFIS.close();
				    }
				    catch(IOException f) {}
				    gnuList.setListData(gnucurves);
				    gnuList.setSelectionInterval(0,gnucurves.size()); 
				    File tocheck4 = new File ( netTextField.getText()+".net.curves" );
				    if( tocheck4.exists() ) tocheck4.delete();
				}
			}
		}
	    });
    
	c.gridx=0;
	c.gridy=2;
	c.gridwidth=3;
	gridBag.setConstraints(calcPanel,c);
	contentPane.add(calcPanel);

	c.gridwidth=1;
	c.gridx=0;
	c.gridy=0;
	c.insets = new Insets(0,0,0,30);
	calcGridBag.setConstraints(netTypeLabel,c);
	calcPanel.add(netTypeLabel);
	c.gridx=0;
	c.gridy=1;
	calcGridBag.setConstraints(gspnRadioButton,c);
	calcPanel.add(gspnRadioButton);
	c.gridx=0;
	c.gridy=2;
	calcGridBag.setConstraints(swnRadioButton,c);
	calcPanel.add(swnRadioButton);

	c.gridx=1;
	c.gridy=0;
	c.insets = new Insets(0,0,0,30);
	calcGridBag.setConstraints(swnTypeLabel,c);
	calcPanel.add(swnTypeLabel);
	c.gridx=1;
	c.gridy=1;
	calcGridBag.setConstraints(ordRadioButton,c);
	calcPanel.add(ordRadioButton);
	c.gridx=1;
	c.gridy=2;
	calcGridBag.setConstraints(symRadioButton,c);
	calcPanel.add(symRadioButton);

	c.gridx=2;
	c.gridy=0;
	calcGridBag.setConstraints(calcTypeLabel,c);
	calcPanel.add(calcTypeLabel);
	c.gridx=2;
	c.gridy=1;
	c.insets = new Insets(0,0,0,30);
	calcGridBag.setConstraints(exactRadioButton,c);
	calcPanel.add(exactRadioButton);
	c.gridx=2;
	c.gridy=2;
	calcGridBag.setConstraints(simRadioButton,c);
	calcPanel.add(simRadioButton);

	c.gridx=3;
	c.gridy=0;
	c.insets = new Insets(0,0,0,0);
	calcGridBag.setConstraints(resultLabel,c);
	calcPanel.add(resultLabel);
	c.gridx=3;
	c.gridy=1;
	calcGridBag.setConstraints(resultTextField,c);
	calcPanel.add(resultTextField);
	c.gridx=3;
	c.gridy=2;
	c.gridwidth=1;
	calcGridBag.setConstraints(calcButton,c);
	calcPanel.add(calcButton);
	c.gridwidth=1;
	
	// Panels for defining gnuplot 
	JPanel gnuPanel = new JPanel();
	gnuPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	gnuPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     gnuPanel.getBorder()));
        GridBagLayout gnuGridBag = new GridBagLayout();
        gnuPanel.setLayout(gnuGridBag);
	JPanel axislimitsPanel = new JPanel();
	axislimitsPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	axislimitsPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     axislimitsPanel.getBorder()));
        GridBagLayout axislimitsGridBag = new GridBagLayout();
        axislimitsPanel.setLayout(axislimitsGridBag);
	JPanel axisnamesPanel = new JPanel();
	axisnamesPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	axisnamesPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     axisnamesPanel.getBorder()));
        GridBagLayout axisnamesGridBag = new GridBagLayout();
        axisnamesPanel.setLayout(axisnamesGridBag);
	JPanel legendPanel = new JPanel();
	legendPanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	legendPanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     legendPanel.getBorder()));
        GridBagLayout legendGridBag = new GridBagLayout();
        legendPanel.setLayout(legendGridBag);
	JPanel linestylePanel = new JPanel();
	linestylePanel.setBorder(BorderFactory.createEmptyBorder(10,10,10,10));
	linestylePanel.setBorder(BorderFactory.createCompoundBorder
			    (BorderFactory.createEtchedBorder(),
			     linestylePanel.getBorder()));
	GridBagLayout linestyleGridBag = new GridBagLayout();
        linestylePanel.setLayout(linestyleGridBag);
	JLabel maingnuLabel = new JLabel("Parameters to create the figure");
	maingnuLabel.setForeground(new Color(0,0,0));
	maingnuLabel.setHorizontalAlignment(JLabel.CENTER);

	c.fill=GridBagConstraints.BOTH;
	c.gridx=0;
	c.gridy=3;
	c.gridwidth=3;
	gridBag.setConstraints(gnuPanel,c);
	contentPane.add(gnuPanel);
	c.gridwidth=1;	
	
	c.fill=GridBagConstraints.BOTH;
	c.gridx=0;
	c.gridwidth=3;
	c.gridy=0;
	gnuGridBag.setConstraints(maingnuLabel,c);
	gnuPanel.add(maingnuLabel);

	c.gridwidth=1;	
	c.weightx=1;
	c.gridy=1;
	c.gridx=0;
	gnuGridBag.setConstraints(axislimitsPanel,c);
	gnuPanel.add(axislimitsPanel);
	c.gridx=1;
	gnuGridBag.setConstraints(legendPanel,c);
	gnuPanel.add(legendPanel);
	c.gridx=2;
	gnuGridBag.setConstraints(linestylePanel,c);
	gnuPanel.add(linestylePanel);
	c.fill=GridBagConstraints.HORIZONTAL;
	c.weightx=0;

	// axislimits panel
	JLabel xminLabel = new JLabel("Minimum of x axis:");
	xminTextField.setBackground(new Color(230,230,230));
	xminTextField.setEnabled(false);
	xminCheckBox.setHorizontalTextPosition(SwingConstants.LEFT);
        xminCheckBox.addItemListener(myListener);
	JLabel xmaxLabel = new JLabel("Maximum of x axis:");
	xmaxTextField.setBackground(new Color(230,230,230));
	xmaxTextField.setEnabled(false);
	xmaxCheckBox.setHorizontalTextPosition(SwingConstants.LEFT);
        xmaxCheckBox.addItemListener(myListener);
	JLabel yminLabel = new JLabel("Minimum of y axis:");
	yminTextField.setBackground(new Color(230,230,230));
	yminTextField.setEnabled(false);
	yminCheckBox.setHorizontalTextPosition(SwingConstants.LEFT);
        yminCheckBox.addItemListener(myListener);
	JLabel ymaxLabel = new JLabel("Maximum of y axis:");
	ymaxTextField.setBackground(new Color(230,230,230));
	ymaxTextField.setEnabled(false);
	ymaxCheckBox.setHorizontalTextPosition(SwingConstants.LEFT);
        ymaxCheckBox.addItemListener(myListener);

	c.gridy=0;
	c.gridx=0;
	axislimitsGridBag.setConstraints(xminLabel,c);
	axislimitsPanel.add(xminLabel);
	c.gridy=0;
	c.gridx=1;
	c.insets = new Insets(0,2,0,0);
	axislimitsGridBag.setConstraints(xminTextField,c);
	axislimitsPanel.add(xminTextField);
	c.gridy=0;
	c.gridx=2;
	c.insets = new Insets(0,10,0,0);
	axislimitsGridBag.setConstraints(xminCheckBox,c);
	axislimitsPanel.add(xminCheckBox);
	c.insets = new Insets(0,0,0,0);
	c.gridy=1;
	c.gridx=0;
	axislimitsGridBag.setConstraints(xmaxLabel,c);
	axislimitsPanel.add(xmaxLabel);
	c.gridy=1;
	c.gridx=1;
	c.insets = new Insets(0,2,0,0);
	axislimitsGridBag.setConstraints(xmaxTextField,c);
	axislimitsPanel.add(xmaxTextField);
	c.gridy=1;
	c.gridx=2;
	c.insets = new Insets(0,10,0,0);
	axislimitsGridBag.setConstraints(xmaxCheckBox,c);
	axislimitsPanel.add(xmaxCheckBox);	
	c.insets = new Insets(0,0,0,0);
	c.gridy=2;
	c.gridx=0;
	axislimitsGridBag.setConstraints(yminLabel,c);
	axislimitsPanel.add(yminLabel);
	c.gridy=2;
	c.gridx=1;
	c.insets = new Insets(0,2,0,0);
	axislimitsGridBag.setConstraints(yminTextField,c);
	axislimitsPanel.add(yminTextField);
	c.gridy=2;
	c.gridx=2;
	c.insets = new Insets(0,10,0,0);
	axislimitsGridBag.setConstraints(yminCheckBox,c);
	axislimitsPanel.add(yminCheckBox);
	c.insets = new Insets(0,0,0,0);
	c.gridy=3;
	c.gridx=0;
	axislimitsGridBag.setConstraints(ymaxLabel,c);
	axislimitsPanel.add(ymaxLabel);
	c.gridy=3;
	c.gridx=1;
	c.insets = new Insets(0,2,0,0);
	axislimitsGridBag.setConstraints(ymaxTextField,c);
	axislimitsPanel.add(ymaxTextField);
	c.gridy=3;
	c.gridx=2;
	c.insets = new Insets(0,10,0,0);
	axislimitsGridBag.setConstraints(ymaxCheckBox,c);
	axislimitsPanel.add(ymaxCheckBox);
	c.insets = new Insets(0,0,0,0);
	
	// legend position panel
	JLabel legendLabel = new JLabel("Position of legend:");
	legendLabel.setForeground(new Color(0,0,0));
	ButtonGroup leftrightGroup = new ButtonGroup();
	leftrightGroup.add(leftRadioButton);	
	leftrightGroup.add(rightRadioButton);
	ButtonGroup topbottomGroup = new ButtonGroup();
	topbottomGroup.add(topRadioButton);	
	topbottomGroup.add(bottomRadioButton);
	
	leftRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);
	rightRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);
	topRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);
	bottomRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);

	c.gridx=0;
	c.gridy=0;
	c.gridwidth=2;
	legendGridBag.setConstraints(legendLabel,c);
	legendPanel.add(legendLabel);
	c.gridwidth=1;
	c.gridx=0;
	c.gridy=1;
	legendGridBag.setConstraints(leftRadioButton,c);
	legendPanel.add(leftRadioButton);
	c.gridx=1;
	c.gridy=1;
	legendGridBag.setConstraints(rightRadioButton,c);
	legendPanel.add(rightRadioButton);
	c.gridx=0;
	c.gridy=2;
	legendGridBag.setConstraints(topRadioButton,c);
	legendPanel.add(topRadioButton);
	c.gridx=1;
	c.gridy=2;
	legendGridBag.setConstraints(bottomRadioButton,c);
	legendPanel.add(bottomRadioButton);
	
	// linestyle panel
	JLabel linestyleLabel = new JLabel("Style of curve:");
	linestyleLabel.setForeground(new Color(0,0,0));
	ButtonGroup styleGroup = new ButtonGroup();
	styleGroup.add(linesRadioButton);
	styleGroup.add(pointsRadioButton);
	styleGroup.add(linespointsRadioButton);
	linesRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);
	pointsRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);
	linespointsRadioButton.setHorizontalTextPosition(SwingConstants.LEFT);

	c.gridx=0;
	c.gridy=0;
	c.gridwidth=1;
	linestyleGridBag.setConstraints(linestyleLabel,c);
	linestylePanel.add(linestyleLabel);
	c.gridwidth=1;
	c.gridx=0;
	c.gridy=1;
	linestyleGridBag.setConstraints(linesRadioButton,c);
	linestylePanel.add(linesRadioButton);
	c.gridx=0;
	c.gridy=2;
	linestyleGridBag.setConstraints(pointsRadioButton,c);
	linestylePanel.add(pointsRadioButton);
	c.gridx=0;
	c.gridy=3;
	linestyleGridBag.setConstraints(linespointsRadioButton,c);
	linestylePanel.add(linespointsRadioButton);

	// gnubuttons and the gnufile
	JLabel gnufileLabel = new JLabel("Name of gnuplot and postscript file:");
	gnufileLabel.setForeground(new Color(0,0,0));
	JLabel gnufileLabel2 = new JLabel("Gnuplot file:");
	gnufileLabel2.setForeground(new Color(0,0,0));
	JLabel gnufileLabel3 = new JLabel("Possible curves:");
	gnufileLabel3.setForeground(new Color(0,0,0));
	JButton gnufileButton = new JButton("Create plot file");
	JScrollPane scrollPane = new JScrollPane(gnufileTextArea,
						 JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
						 JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
	gnuList.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
        JScrollPane listScrollPane = new JScrollPane(gnuList);
	gnufileButton.addActionListener(new ActionListener() 
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    String command;
		    int i,ok=1,ns=0;

		    File tocheck3 = new File ( netTextField.getText()+".results" );
		    if( !tocheck3.exists() && ok!=0)
			{
			    JOptionPane.showMessageDialog(frame, 
                               "No results are found. \nPerform computations first.");
			    ok=0;
			}
		    for(i=0;i<gnucurves.size();i++)
			if( gnuList.isSelectedIndex(i)) ns++;
		    if( ok!=0 && ns==0)
			{
			    JOptionPane.showMessageDialog(frame, 
                               "No curves are selected.");
			    ok=0;
			}

		    if(ok==1)
			{
			    try {
				FileOutputStream curvesFile = 
				    new FileOutputStream(netTextField.getText() + ".net.curves");
				PrintWriter curvesWriter = 
				    new PrintWriter(curvesFile);
				for(i=0;i<gnucurves.size();i++)
				    if( gnuList.isSelectedIndex(i)) 
					curvesWriter.print(gnucurves.get(i) + "\n");
				curvesWriter.close();
			    }
			    catch (FileNotFoundException f) {}
			    
			    command = "xterm -sl 1000 -sb -e tcsh " + greathome + "/multisolve/GnuPlot ";
			    command += netTextField.getText() + " ";
			    command += gnufileTextField.getText() + " ";
			    if( xminCheckBox.isSelected() ) {
				command += "auto ";
			    }
			    else {
				command += xminTextField.getText() + " ";
			    }
			    if( xmaxCheckBox.isSelected() ) {
				command += "auto ";
			    }
			    else {
				command += xmaxTextField.getText() + " ";
			    }
			    if( yminCheckBox.isSelected() ) {
				command += "auto ";
			    }
			    else {
				command += yminTextField.getText() + " ";
			    }
			    if( ymaxCheckBox.isSelected() ) {
				command += "auto ";
			    }
			    else {
				command += ymaxTextField.getText() + " ";
			    }
			    if( linesRadioButton.isSelected() ) 
				command += "l ";
			    if( pointsRadioButton.isSelected() ) 
				command += "p ";
			    if( linespointsRadioButton.isSelected() ) 
				command += "lp ";
			    if( leftRadioButton.isSelected() ) 
				command += "left ";
			    if( rightRadioButton.isSelected() ) 
				command += "right ";
			    if( topRadioButton.isSelected() ) 
				command += "top ";
			    if( bottomRadioButton.isSelected() ) 
				command += "bottom ";
			    
			    gnufileTextArea.setText("");
			    execProcess(command);
			    try {
				FileInputStream gnuFIS = new FileInputStream(gnufileTextField.getText() + ".gnu");
				BufferedReader gnuBR = new BufferedReader(new InputStreamReader(gnuFIS));
				while(gnuBR.ready())
				    gnufileTextArea.append(gnuBR.readLine() + "\n");
				gnuFIS.close();
			    }
			    catch(IOException f) {}
			}
		}
	    });
	JButton postscriptButton = new JButton("Make postscript");
	postscriptButton.addActionListener(new ActionListener() 
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    int ok=1;
		
		    File tocheck3 = new File ( gnufileTextField.getText()+".gnu" );
		    if( !tocheck3.exists() && ok!=0)
			{
			    JOptionPane.showMessageDialog(frame, 
	         	       "No gnu-file found. \n Create gnu-file first.");
			    ok=0;
			}
    
		    if(ok==1)
			{
			    // write parameters to files
			    try 
				{
				    FileOutputStream gnuplotFile = 
					new FileOutputStream(gnufileTextField.getText()+".gnu");
				    PrintWriter gnuplotWriter = 
					new PrintWriter(gnuplotFile);
				    gnuplotWriter.print(gnufileTextArea.getText());
				    gnuplotWriter.close();
				}
			    catch (FileNotFoundException f) {}
			    execProcess("gnuplot " +  gnufileTextField.getText() + ".gnu");
			}
		}
	    });

	c.insets = new Insets(10,0,0,0);
	c.fill=GridBagConstraints.BOTH;
	c.gridy=2;
	c.gridx=0;
	gnuGridBag.setConstraints(gnufileLabel,c);
	gnuPanel.add(gnufileLabel);
	c.insets = new Insets(0,0,0,0);
	c.gridy=3;
	c.gridx=0;
	gnuGridBag.setConstraints(gnufileTextField,c);
	gnuPanel.add(gnufileTextField);
	c.gridy=4;
	c.gridx=0;
	gnuGridBag.setConstraints(gnufileButton,c);
	gnuPanel.add(gnufileButton);
	c.gridy=5;
	c.gridx=0;
	gnuGridBag.setConstraints(gnufileLabel3,c);
	gnuPanel.add(gnufileLabel3);
	c.gridy=6;
	c.gridx=0;
	c.weighty=1;
	gnuGridBag.setConstraints(listScrollPane,c);
	gnuPanel.add(listScrollPane);
	c.weighty=0;
	c.gridy=7;
	c.gridx=0;
	gnuGridBag.setConstraints(postscriptButton,c);
	gnuPanel.add(postscriptButton);
	c.gridy=2;
	c.gridx=1;
	c.insets = new Insets(10,10,0,0);
	gnuGridBag.setConstraints(gnufileLabel2,c);
	gnuPanel.add(gnufileLabel2);
	c.gridy=3;
	c.gridx=1;
	c.gridwidth=2;
	c.gridheight=5;
	gnuGridBag.setConstraints(scrollPane,c);
	gnuPanel.add(scrollPane);
	c.gridheight=1;
	c.insets = new Insets(0,0,0,0);

    }

    class CheckBoxListener implements ItemListener {
        public void itemStateChanged(ItemEvent e) {
            Object source = e.getItemSelectable();

	    if (source == xminCheckBox) {
		if (e.getStateChange() == ItemEvent.DESELECTED) {
		    xminTextField.setBackground(Color.white);
		    xminTextField.setEnabled(true);
		    xminTextField.requestFocus();
		}
		else {
		    xminTextField.setBackground(new Color(230,230,230));
		    xminTextField.setEnabled(false);
		}
	    }
	    if (source == xmaxCheckBox) {
		if (e.getStateChange() == ItemEvent.DESELECTED) {
		    xmaxTextField.setBackground(Color.white);
		    xmaxTextField.setEnabled(true);
		    xmaxTextField.requestFocus();
		}
		else {
		    xmaxTextField.setBackground(new Color(230,230,230));
		    xmaxTextField.setEnabled(false);
		}
	    }
	    if (source == yminCheckBox) {
		if (e.getStateChange() == ItemEvent.DESELECTED) {
		    yminTextField.setBackground(Color.white);
		    yminTextField.setEnabled(true);
		    yminTextField.requestFocus();
		}
		else {
		    yminTextField.setBackground(new Color(230,230,230));
		    yminTextField.setEnabled(false);
		}
	    }
	    if (source == ymaxCheckBox) {
		if (e.getStateChange() == ItemEvent.DESELECTED) {
		    ymaxTextField.setBackground(Color.white);
		    ymaxTextField.setEnabled(true);
		    ymaxTextField.requestFocus();
		}
		else {
		    ymaxTextField.setBackground(new Color(230,230,230));
		    ymaxTextField.setEnabled(false);
		}
	    }
	    if (source == timeCheckBox) {
		if (e.getStateChange() == ItemEvent.DESELECTED) {
		    xaxisTextField.setBackground(Color.white);
		    xaxisTextField.setEnabled(true);
		    xaxisTextField.setText("");
		    xaxisTextField.requestFocus();
		    exactRadioButton.setEnabled(true);
		    simRadioButton.setEnabled(true);
		}
		else {
		    xaxisTextField.setBackground(new Color(230,230,230));
		    xaxisTextField.setEnabled(false);
		    xaxisTextField.setText("Time");
		    exactRadioButton.setEnabled(false);
		    exactRadioButton.setSelected(true);
		    simRadioButton.setEnabled(false);
		}
	    }
	    if (source == gspnRadioButton) {
		if (e.getStateChange() == ItemEvent.DESELECTED) {
		    ordRadioButton.setEnabled(true);
		    symRadioButton.setEnabled(true);
		}
		else {
		    ordRadioButton.setSelected(true);
		    ordRadioButton.setEnabled(false);
		    symRadioButton.setEnabled(false);
		}
	    }
	    if (source == exactRadioButton) {
	      if (e.getStateChange() == ItemEvent.DESELECTED) {
		    initTransTextField.setBackground(Color.white);
		    initTransTextField.setEnabled(true);
		    batchSpTextField.setBackground(Color.white);
		    batchSpTextField.setEnabled(true);
		    minLengthTextField.setBackground(Color.white);
		    minLengthTextField.setEnabled(true);
		    maxLengthTextField.setBackground(Color.white);
		    maxLengthTextField.setEnabled(true);
		    seedTextField.setBackground(Color.white);
		    seedTextField.setEnabled(true);
		    accuracyTextField.setBackground(Color.white);
		    accuracyTextField.setEnabled(true);
		    confList.setEnabled(true);
		}
		else {
		    initTransTextField.setBackground(new Color(230,230,230));
		    initTransTextField.setEnabled(false);
		    batchSpTextField.setBackground(new Color(230,230,230));
		    batchSpTextField.setEnabled(false);
		    minLengthTextField.setBackground(new Color(230,230,230));
		    minLengthTextField.setEnabled(false);
		    maxLengthTextField.setBackground(new Color(230,230,230));
		    maxLengthTextField.setEnabled(false);
		    seedTextField.setBackground(new Color(230,230,230));
		    seedTextField.setEnabled(false);
		    accuracyTextField.setBackground(new Color(230,230,230));
		    accuracyTextField.setEnabled(false);
		    confList.setEnabled(false);
		}
	    }
	}
    }
    public static void main(String[] args) 
    {
	//System.out.println(args[0]);
        JFrame frame = new MultiSolve(args[0],args[1]);
        frame.addWindowListener(new WindowAdapter() 
	    {
		public void windowClosing(WindowEvent e) 
		{
		    System.exit(0);
		}
	    });
	
        frame.pack();
        frame.setVisible(true);
    }
}

