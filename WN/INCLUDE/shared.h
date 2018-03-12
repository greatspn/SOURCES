extern void add_instance();
extern void init_enabling_phase();
extern void setup_cartesian_product_of_tr();
extern void project_element_to_instance();
extern void test_old_instances();
extern int new_instances_generated();
extern int is_in_function();
extern int project_function_on_domain();
extern int project_token_to_instance();

extern void check_for_enabling();
extern void adjust_enable_degree();
extern int pre_check();
extern Node_p get_place_list();

extern Event_p new_enabled_list;
extern Event_p old_enabled_list;
