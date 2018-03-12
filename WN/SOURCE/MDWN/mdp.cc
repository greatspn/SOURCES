// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "mdp.h"


namespace graphMDP {
	
	
StationaryOption make_stationary_option(bool is_infinite_, InfiniteCriteriumType infinite_criterium_, DecompositionAlgorithmType decomposition_algorithm_, unsigned int horizon_)
{
	StationaryOption stationary_option;
	stationary_option.is_infinite = is_infinite_;
	stationary_option.infinite_criterium = infinite_criterium_;
	stationary_option.decomposition_algorithm = decomposition_algorithm_;
	stationary_option.horizon = horizon_;
	return stationary_option;
}


Glib::RefPtr<mdp> CreateMDP(Glib::ustring const & data_file, StationaryOption stationary_option) throw(xmlpp::exception,Exception)
{
	Glib::RefPtr<mdp> new_mdp;
	
	xmlpp::DomParser xml_parser;
	xml_parser.set_validate(true);
        xml_parser.set_substitute_entities();
	xml_parser.parse_file(data_file);
	xmlpp::Document * mdp_doc = xml_parser.get_document();


	if (mdp_doc == NULL)
	{
		xmlException xml_mdp_error("XML document unvalidate");
		throw xml_mdp_error;
	}
	
	xmlpp::Element * mdp_root_node = mdp_doc->get_root_node();

	xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();

	xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
	xmlpp::Element * xml_states_element = NULL;
	xmlpp::Element * xml_regions_element = NULL;
	xmlpp::Element * xml_actions_element = NULL;
	

	
	for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
	{
		if ((*Imdp_root_node_children)->get_name() == "STATES")
		{
			xml_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "REGIONS")
		{
			xml_regions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "ACTIONS")
		{
			xml_actions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
	}
	
	if (mdp_root_node->get_attribute("stationary")->get_value() == "no") // unstationary MDP
	{
		xmlpp::Attribute * horizon_attribute = mdp_root_node->get_attribute("horizon");
		
		if (horizon_attribute == NULL)
		{
			xmlException xml_mdp_error("the horizon is not given.");
			throw xml_mdp_error;
		}
		
		new_mdp = UnstationaryMDP::create(*xml_states_element, *xml_actions_element, *horizon_attribute);
	}
	else // stationary MDP
	{
		if (stationary_option.is_infinite) // infinite MDP
		{
			xmlpp::Attribute * decomposable_attribute = mdp_root_node->get_attribute("decomposable");
			bool decomposable = false;
			
			if (decomposable_attribute == NULL) // not decomposable
			{
				decomposable = false;
			}
			else
			{
				if (decomposable_attribute->get_value() == "no")
				{
					decomposable = false;
				}
				else
				{
					decomposable = true;
				}
			}
			
			if (!decomposable)
			{
				switch (stationary_option.infinite_criterium)
				{
					case DISCOUNTED :
						new_mdp = DiscountedMDP::create(*xml_states_element, *xml_actions_element);
						break;
					
					case AVERAGE_REWARD :
						new_mdp = AverageRewardMDP::create(*xml_states_element, *xml_actions_element);
						break;
				}
			}
			else
			{
				switch (stationary_option.decomposition_algorithm)
				{
					case GRID_DECOMPOSITION :
						new_mdp = DecomposableMDP<GridRegion>::create(*xml_regions_element, *xml_actions_element);
						break;
					
					case LINEAR_PROGRAMMING_DECOMPOSITION :
						new_mdp = DecomposableMDP<LinearProgrammingRegion>::create(*xml_regions_element, *xml_actions_element);
						break;
				}
			}
		}
		else // finite stationary MDP
		{
			new_mdp = FiniteStationaryMDP::create(*xml_states_element, *xml_actions_element, stationary_option.horizon);
		}
	}
	
	return new_mdp;
}




Glib::RefPtr<mdp> CreateMDP(Glib::ustring const & data_file, StationaryOption (*stationary_criterion_query) ()) throw(xmlpp::exception,Exception)
{
	Glib::RefPtr<mdp> new_mdp;
	
	xmlpp::DomParser xml_parser;
	xml_parser.set_validate(true);
	xml_parser.parse_file(data_file);
	xmlpp::Document * mdp_doc = xml_parser.get_document();
	
	if (mdp_doc == NULL)
	{
		xmlException xml_mdp_error("XML document unvalidate");
		throw xml_mdp_error;
	}
	
	xmlpp::Element * mdp_root_node = mdp_doc->get_root_node();
	xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();
	xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
	xmlpp::Element * xml_states_element = NULL;
	xmlpp::Element * xml_regions_element = NULL;
	xmlpp::Element * xml_actions_element = NULL;
	
	for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
	{
		if ((*Imdp_root_node_children)->get_name() == "STATES")
		{
			xml_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "REGIONS")
		{
			xml_regions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "ACTIONS")
		{
			xml_actions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
	}
	
	if (mdp_root_node->get_attribute("stationary")->get_value() == "no") // unstationary MDP
	{
		xmlpp::Attribute * horizon_attribute = mdp_root_node->get_attribute("horizon");
		
		if (horizon_attribute == NULL)
		{
			xmlException xml_mdp_error("the horizon is not given.");
			throw xml_mdp_error;
		}
		
		new_mdp = UnstationaryMDP::create(*xml_states_element, *xml_actions_element, *horizon_attribute);
	}
	else // stationary MDP
	{
		StationaryOption stationary_option;
		
		if (stationary_criterion_query == NULL)
		{
			stationary_option = make_stationary_option(true, DISCOUNTED);
		}
		else
		{
			stationary_option = stationary_criterion_query();
		}
		
		if (stationary_option.is_infinite) // infinite MDP
		{
			xmlpp::Attribute * decomposable_attribute = mdp_root_node->get_attribute("decomposable");
			bool decomposable = false;
			
			if (decomposable_attribute == NULL) // not decomposable
			{
				decomposable = false;
			}
			else
			{
				if (decomposable_attribute->get_value() == "no")
				{
					decomposable = false;
				}
				else
				{
					decomposable = true;
				}
			}
			
			if (!decomposable)
			{
				switch (stationary_option.infinite_criterium)
				{
					case DISCOUNTED :
						new_mdp = DiscountedMDP::create(*xml_states_element, *xml_actions_element);
						break;
					
					case AVERAGE_REWARD :
						new_mdp = AverageRewardMDP::create(*xml_states_element, *xml_actions_element);
						break;
				}
			}
			else
			{
				switch (stationary_option.decomposition_algorithm)
				{
					case GRID_DECOMPOSITION :
						new_mdp = DecomposableMDP<GridRegion>::create(*xml_regions_element, *xml_actions_element);
						break;
					
					case LINEAR_PROGRAMMING_DECOMPOSITION :
						new_mdp = DecomposableMDP<LinearProgrammingRegion>::create(*xml_regions_element, *xml_actions_element);
						break;
				}
			}
		}
		else // finite stationary MDP
		{
			new_mdp = FiniteStationaryMDP::create(*xml_states_element, *xml_actions_element, stationary_option.horizon);
		}
	}
	
	return new_mdp;
}




std::pair<Glib::RefPtr<mdp> , Glib::RefPtr<drawable_mdp> > CreateDrawableMDP(Glib::ustring const & data_file, StationaryOption stationary_option) throw(xmlpp::exception,Exception)
{
	Glib::RefPtr<mdp> new_mdp;
	Glib::RefPtr<drawable_mdp> new_drawable_mdp;
	
	xmlpp::DomParser xml_parser;
	xml_parser.set_validate(true);
	xml_parser.parse_file(data_file);
	xmlpp::Document * mdp_doc = xml_parser.get_document();
	
	if (mdp_doc == NULL)
	{
		xmlException xml_mdp_error("XML document unvalidate");
		throw xml_mdp_error;
	}
	
	xmlpp::Element * mdp_root_node = mdp_doc->get_root_node();
	xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();
	xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
	xmlpp::Element * xml_states_element = NULL;
	xmlpp::Element * xml_actions_element = NULL;
	
	for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
	{
		if ((*Imdp_root_node_children)->get_name() == "STATES")
		{
			xml_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "ACTIONS")
		{
			xml_actions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
	}
	
	if (mdp_root_node->get_attribute("stationary")->get_value() == "no") // unstationary MDP
	{
		xmlpp::Attribute * horizon_attribute = mdp_root_node->get_attribute("horizon");
		
		if (horizon_attribute == NULL)
		{
			xmlException xml_mdp_error("the horizon is not given.");
			throw xml_mdp_error;
		}
		
		try
		{			
			new_mdp = UnstationaryMDP::create(*xml_states_element, *xml_actions_element, *horizon_attribute);
		}
		catch(...)
		{
			throw;
		}
		
		try
		{
			new_drawable_mdp = UnstationaryDrawableMDP::create(Glib::RefPtr<UnstationaryMDP>::cast_dynamic(new_mdp), *xml_states_element);
		}
		catch(...)
		{
			throw;
		}
	}
	else // stationary MDP
	{
		if (stationary_option.is_infinite) // infinite MDP
		{
			switch (stationary_option.infinite_criterium)
			{
				case DISCOUNTED :
					try
					{			
						new_mdp = DiscountedMDP::create(*xml_states_element, *xml_actions_element);
					}
					catch(...)
					{
						throw;
					}
					
					try
					{
						new_drawable_mdp = InfiniteDrawableMDP<DiscountedMDP>::create(Glib::RefPtr<DiscountedMDP>::cast_dynamic(new_mdp), *xml_states_element);
					}
					catch(...)
					{
						throw;
					}
					
					break;
				
				case AVERAGE_REWARD :
					try
					{			
						new_mdp = AverageRewardMDP::create(*xml_states_element, *xml_actions_element);
					}
					catch(...)
					{
						throw;
					}
					
					try
					{
						new_drawable_mdp = InfiniteDrawableMDP<AverageRewardMDP>::create(Glib::RefPtr<AverageRewardMDP>::cast_dynamic(new_mdp), *xml_states_element);
					}
					catch(...)
					{
						throw;
					}
					
					break;
			}
		}
		else // finite stationary MDP
		{
			try
			{			
				new_mdp = FiniteStationaryMDP::create(*xml_states_element, *xml_actions_element, stationary_option.horizon);
			}
			catch(...)
			{
				throw;
			}
			
			try
			{
				new_drawable_mdp = FiniteStationaryDrawableMDP::create(Glib::RefPtr<FiniteStationaryMDP>::cast_dynamic(new_mdp), *xml_states_element);
			}
			catch(...)
			{
				throw;
			}
		}
	}
	
	return std::make_pair(new_mdp, new_drawable_mdp);
}




std::pair<Glib::RefPtr<mdp> , Glib::RefPtr<drawable_mdp> > CreateDrawableMDP(Glib::ustring const & data_file, StationaryOption (*stationary_criterion_query) ()) throw(xmlpp::exception,Exception)
{
	Glib::RefPtr<mdp> new_mdp;
	Glib::RefPtr<drawable_mdp> new_drawable_mdp;
	
	xmlpp::DomParser xml_parser;
	xml_parser.set_validate(true);
	xml_parser.parse_file(data_file);
	xmlpp::Document * mdp_doc = xml_parser.get_document();
	
	if (mdp_doc == NULL)
	{
		xmlException xml_mdp_error("XML document unvalidate");
		throw xml_mdp_error;
	}
	
	xmlpp::Element * mdp_root_node = mdp_doc->get_root_node();
	xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();
	xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
	xmlpp::Element * xml_states_element = NULL;
	xmlpp::Element * xml_actions_element = NULL;
	
	for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
	{
		if ((*Imdp_root_node_children)->get_name() == "STATES")
		{
			xml_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "ACTIONS")
		{
			xml_actions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
	}
	
	if (mdp_root_node->get_attribute("stationary")->get_value() == "no") // unstationary MDP
	{
		xmlpp::Attribute * horizon_attribute = mdp_root_node->get_attribute("horizon");
		
		if (horizon_attribute == NULL)
		{
			xmlException xml_mdp_error("the horizon is not given.");
			throw xml_mdp_error;
		}
		
		try
		{			
			new_mdp = UnstationaryMDP::create(*xml_states_element, *xml_actions_element, *horizon_attribute);
		}
		catch(...)
		{
			throw;
		}
		
		try
		{
			new_drawable_mdp = UnstationaryDrawableMDP::create(Glib::RefPtr<UnstationaryMDP>::cast_dynamic(new_mdp), *xml_states_element);
		}
		catch(...)
		{
			throw;
		}
	}
	else // stationary MDP
	{
		StationaryOption stationary_option;
		
		if (stationary_criterion_query == NULL)
		{
			stationary_option = make_stationary_option(true, DISCOUNTED);
		}
		else
		{
			stationary_option = stationary_criterion_query();
		}
		
		if (stationary_option.is_infinite) // infinite MDP
		{
			switch (stationary_option.infinite_criterium)
			{
				case DISCOUNTED :
					try
					{			
						new_mdp = DiscountedMDP::create(*xml_states_element, *xml_actions_element);
					}
					catch(...)
					{
						throw;
					}
					
					try
					{
						new_drawable_mdp = InfiniteDrawableMDP<DiscountedMDP>::create(Glib::RefPtr<DiscountedMDP>::cast_dynamic(new_mdp), *xml_states_element);
					}
					catch(...)
					{
						throw;
					}
					
					break;
				
				case AVERAGE_REWARD :
					try
					{			
						new_mdp = AverageRewardMDP::create(*xml_states_element, *xml_actions_element);
					}
					catch(...)
					{
						throw;
					}
					
					try
					{
						new_drawable_mdp = InfiniteDrawableMDP<AverageRewardMDP>::create(Glib::RefPtr<AverageRewardMDP>::cast_dynamic(new_mdp), *xml_states_element);
					}
					catch(...)
					{
						throw;
					}
					
					break;
			}
		}
		else // finite stationary MDP
		{
			try
			{			
				new_mdp = FiniteStationaryMDP::create(*xml_states_element, *xml_actions_element, stationary_option.horizon);
			}
			catch(...)
			{
				throw;
			}
			
			try
			{
				new_drawable_mdp = FiniteStationaryDrawableMDP::create(Glib::RefPtr<FiniteStationaryMDP>::cast_dynamic(new_mdp), *xml_states_element);
			}
			catch(...)
			{
				throw;
			}
		}
	}
	
	return std::make_pair(new_mdp, new_drawable_mdp);
}




Glib::RefPtr<maze_2d> CreateMaze2D(Glib::ustring const & data_file, StationaryOption stationary_option) throw(xmlpp::exception,Exception)
{
	Glib::RefPtr<maze_2d> new_maze_2d;
	
	xmlpp::DomParser xml_parser;
	xml_parser.set_validate(true);
	xml_parser.parse_file(data_file);
	xmlpp::Document * mdp_doc = xml_parser.get_document();
	
	if (mdp_doc == NULL)
	{
		xmlException xml_mdp_error("XML document unvalidate");
		throw xml_mdp_error;
	}
	
	xmlpp::Element * mdp_root_node = mdp_doc->get_root_node();
	xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();
	xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
	xmlpp::Element * xml_dimensions_element = NULL;
	xmlpp::Element * xml_forbidden_states_element = NULL;
	xmlpp::Element * xml_forbidden_transitions_element = NULL;
	
	for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
	{
		if ((*Imdp_root_node_children)->get_name() == "DIMENSIONS")
		{
			xml_dimensions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "FORBIDDEN_STATES")
		{
			xml_forbidden_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "FORBIDDEN_TRANSITIONS")
		{
			xml_forbidden_transitions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
	}
	
	xmlpp::Attribute * with_diagonal_actions_attribute = mdp_root_node->get_attribute("with_diagonal_actions");
	
	if (mdp_root_node->get_attribute("stationary")->get_value() == "no") // unstationary 2D Maze
	{
		xmlpp::Attribute * horizon_attribute = mdp_root_node->get_attribute("horizon");
		
		if (horizon_attribute == NULL)
		{
			xmlException xml_mdp_error("the horizon is not given.");
			throw xml_mdp_error;
		}
		
		std::list<xmlpp::Element const *> xml_deviation_probabilities_elements;
		std::list<xmlpp::Element const *> xml_rewards_elements;
		
		for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
		{
			if ((*Imdp_root_node_children)->get_name() == "DEVIATION_PROBABILITIES")
			{
				xml_deviation_probabilities_elements.push_back(dynamic_cast<xmlpp::Element const *>(*Imdp_root_node_children));
			}
			
			if ((*Imdp_root_node_children)->get_name() == "REWARDS")
			{
				xml_rewards_elements.push_back(dynamic_cast<xmlpp::Element const *>(*Imdp_root_node_children));
			}
		}
		
		new_maze_2d = UnstationaryMaze2D::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, xml_deviation_probabilities_elements, xml_rewards_elements, *with_diagonal_actions_attribute, *horizon_attribute);
	}
	else // stationary 2D Maze
	{
		xmlpp::Element * xml_deviation_probabilities_element = NULL;
		xmlpp::Element * xml_rewards_element = NULL;
		
		for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
		{
			if ((*Imdp_root_node_children)->get_name() == "DEVIATION_PROBABILITIES")
			{
				xml_deviation_probabilities_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
			}
			
			if ((*Imdp_root_node_children)->get_name() == "REWARDS")
			{
				xml_rewards_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
			}
		}
		
		if (stationary_option.is_infinite) // infinite 2D Maze
		{
			xmlpp::Attribute * decomposable_attribute = mdp_root_node->get_attribute("decomposable");
			bool decomposable = false;
			
			if (decomposable_attribute == NULL) // not decomposable
			{
				decomposable = false;
			}
			else
			{
				if (decomposable_attribute->get_value() == "no")
				{
					decomposable = false;
				}
				else
				{
					decomposable = true;
				}
			}
			
			if (!decomposable)
			{
				switch (stationary_option.infinite_criterium)
				{
					case DISCOUNTED :
						new_maze_2d = InfiniteMaze2D<DiscountedMDP>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *with_diagonal_actions_attribute);
						break;
					
					case AVERAGE_REWARD :
						new_maze_2d = InfiniteMaze2D<AverageRewardMDP>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *with_diagonal_actions_attribute);
						break;
				}
			}
			else
			{
				xmlpp::Element * xml_regions_element = NULL;
				
				for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
				{
					if ((*Imdp_root_node_children)->get_name() == "REGIONS")
					{
						xml_regions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
					}
				}
				
				switch (stationary_option.decomposition_algorithm)
				{
					case GRID_DECOMPOSITION :
						new_maze_2d = DecomposableMaze2D<GridRegion>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *xml_regions_element, *with_diagonal_actions_attribute);
						break;
					
					case LINEAR_PROGRAMMING_DECOMPOSITION :
						new_maze_2d = DecomposableMaze2D<LinearProgrammingRegion>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *xml_regions_element, *with_diagonal_actions_attribute);
						break;
				}
			}
		}
		else // finite stationary MDP
		{
			new_maze_2d = FiniteStationaryMaze2D::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *with_diagonal_actions_attribute, stationary_option.horizon);
		}
	}
	
	return new_maze_2d;
}




Glib::RefPtr<maze_2d> CreateMaze2D(Glib::ustring const & data_file, StationaryOption (*stationary_criterion_query) ()) throw(xmlpp::exception,Exception)
{
	Glib::RefPtr<maze_2d> new_maze_2d;
	
	xmlpp::DomParser xml_parser;
	xml_parser.set_validate(true);
	xml_parser.parse_file(data_file);
	xmlpp::Document * mdp_doc = xml_parser.get_document();
	
	if (mdp_doc == NULL)
	{
		xmlException xml_mdp_error("XML document unvalidate");
		throw xml_mdp_error;
	}
	
	xmlpp::Element * mdp_root_node = mdp_doc->get_root_node();
	xmlpp::Node::NodeList mdp_root_node_children = mdp_root_node->get_children();
	xmlpp::Node::NodeList::const_iterator Imdp_root_node_children;
	xmlpp::Element * xml_dimensions_element = NULL;
	xmlpp::Element * xml_forbidden_states_element = NULL;
	xmlpp::Element * xml_forbidden_transitions_element = NULL;
	
	for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
	{
		if ((*Imdp_root_node_children)->get_name() == "DIMENSIONS")
		{
			xml_dimensions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "FORBIDDEN_STATES")
		{
			xml_forbidden_states_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
		
		if ((*Imdp_root_node_children)->get_name() == "FORBIDDEN_TRANSITIONS")
		{
			xml_forbidden_transitions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
		}
	}
	
	xmlpp::Attribute * with_diagonal_actions_attribute = mdp_root_node->get_attribute("with_diagonal_actions");
	
	if (mdp_root_node->get_attribute("stationary")->get_value() == "no") // unstationary 2D Maze
	{
		xmlpp::Attribute * horizon_attribute = mdp_root_node->get_attribute("horizon");
		
		if (horizon_attribute == NULL)
		{
			xmlException xml_mdp_error("the horizon is not given.");
			throw xml_mdp_error;
		}
		
		std::list<xmlpp::Element const *> xml_deviation_probabilities_elements;
		std::list<xmlpp::Element const *> xml_rewards_elements;
		
		for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
		{
			if ((*Imdp_root_node_children)->get_name() == "DEVIATION_PROBABILITIES")
			{
				xml_deviation_probabilities_elements.push_back(dynamic_cast<xmlpp::Element const *>(*Imdp_root_node_children));
			}
			
			if ((*Imdp_root_node_children)->get_name() == "REWARDS")
			{
				xml_rewards_elements.push_back(dynamic_cast<xmlpp::Element const *>(*Imdp_root_node_children));
			}
		}
		
		new_maze_2d = UnstationaryMaze2D::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, xml_deviation_probabilities_elements, xml_rewards_elements, *with_diagonal_actions_attribute, *horizon_attribute);
	}
	else // stationary 2D Maze
	{
		xmlpp::Element * xml_deviation_probabilities_element = NULL;
		xmlpp::Element * xml_rewards_element = NULL;
		
		for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
		{
			if ((*Imdp_root_node_children)->get_name() == "DEVIATION_PROBABILITIES")
			{
				xml_deviation_probabilities_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
			}
			
			if ((*Imdp_root_node_children)->get_name() == "REWARDS")
			{
				xml_rewards_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
			}
		}
		
		StationaryOption stationary_option;
		
		if (stationary_criterion_query == NULL)
		{
			stationary_option = make_stationary_option(true, DISCOUNTED);
		}
		else
		{
			stationary_option = stationary_criterion_query();
		}
		
		if (stationary_option.is_infinite) // infinite 2D Maze
		{
			xmlpp::Attribute * decomposable_attribute = mdp_root_node->get_attribute("decomposable");
			bool decomposable = false;
			
			if (decomposable_attribute == NULL) // not decomposable
			{
				decomposable = false;
			}
			else
			{
				if (decomposable_attribute->get_value() == "no")
				{
					decomposable = false;
				}
				else
				{
					decomposable = true;
				}
			}
			
			if (!decomposable)
			{
				switch (stationary_option.infinite_criterium)
				{
					case DISCOUNTED :
						new_maze_2d = InfiniteMaze2D<DiscountedMDP>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *with_diagonal_actions_attribute);
						break;
					
					case AVERAGE_REWARD :
						new_maze_2d = InfiniteMaze2D<AverageRewardMDP>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *with_diagonal_actions_attribute);
						break;
				}
			}
			else
			{
				xmlpp::Element * xml_regions_element = NULL;
				
				for (Imdp_root_node_children = mdp_root_node_children.begin() ; Imdp_root_node_children != mdp_root_node_children.end() ; ++Imdp_root_node_children)
				{
					if ((*Imdp_root_node_children)->get_name() == "REGIONS")
					{
						xml_regions_element = dynamic_cast<xmlpp::Element *>(*Imdp_root_node_children);
					}
				}
				
				switch (stationary_option.decomposition_algorithm)
				{
					case GRID_DECOMPOSITION :
						new_maze_2d = DecomposableMaze2D<GridRegion>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *xml_regions_element, *with_diagonal_actions_attribute);
						break;
					
					case LINEAR_PROGRAMMING_DECOMPOSITION :
						new_maze_2d = DecomposableMaze2D<LinearProgrammingRegion>::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *xml_regions_element, *with_diagonal_actions_attribute);
						break;
				}
			}
		}
		else // finite stationary MDP
		{
			new_maze_2d = FiniteStationaryMaze2D::create(*xml_dimensions_element, *xml_forbidden_states_element, *xml_forbidden_transitions_element, *xml_deviation_probabilities_element, *xml_rewards_element, *with_diagonal_actions_attribute, stationary_option.horizon);
		}
	}
	
	return new_maze_2d;
}


}
