/* C++ wrapper experiement. */

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <cassert>

#include <tskit/tables.h>

using namespace std;

void
check_error(int val)
{
    if (val < 0) {
        std::ostringstream o;
        o << tsk_strerror(val);
        throw std::runtime_error(o.str());
    }
}

class NodeTable
{

    private:
        tsk_node_table_t *table;
        const bool allocated_locally;

        tsk_node_table_t* allocate(tsk_node_table_t * the_table)
        {
            if (the_table != nullptr){ return the_table; }

            using node_table_ptr = std::unique_ptr<tsk_node_table_t,void(*)(tsk_node_table_t*)>;
            node_table_ptr t(new tsk_node_table_t{},[](tsk_node_table_t* nt){delete nt;});
            if (t.get() == NULL) {
                throw std::runtime_error("Out of memory");
            }
            int ret = tsk_node_table_init(t.get(), 0);
            // NOTE: check error is dangerous here.  If you don't
            // use a smart pointer above, you will get a leak.
            check_error(ret);
            return t.release();
        }

    public:
        explicit NodeTable(tsk_node_table_t *the_table) : table(allocate(the_table)), allocated_locally(the_table != table)
        {
            std::cout << "table constructor" << endl;
        }

        ~NodeTable()
        {
            if (allocated_locally && table != NULL) {
                tsk_node_table_free(table);
                delete table;
            }
        }


        tsk_id_t add_row(tsk_flags_t flags, double time) /* and more params */
        {
            tsk_id_t ret = tsk_node_table_add_row(table, flags, time, TSK_NULL,
                    TSK_NULL, NULL, 0);
            check_error(ret);
            return ret;
        }

        tsk_size_t get_num_rows(void) const
        {
            return table->num_rows;
        }
        /* Etc */
};

class TableCollection
{

    private:
        std::unique_ptr<tsk_table_collection_t, void(*)(tsk_table_collection_t*)> tables;

    public:
        NodeTable nodes;

        explicit TableCollection(double sequence_length) : tables(new tsk_table_collection_t{},[](tsk_table_collection_t *t){delete t;}),
                 nodes(&tables->nodes)
        {
            if (tables == nullptr) {
                throw std::runtime_error("Out of memory");
            }
            int ret = tsk_table_collection_init(tables.get(), 0);
            // NOTE: without the smart pointer, this is a memory leak 
            // waiting to happen.  The destructor is NOT called 
            // if there is an exception from a constructor.
            check_error(ret);
            tables->sequence_length = sequence_length;
        }

        ~TableCollection()
        {
            if (tables != nullptr) {
                tsk_table_collection_free(tables.get());
            }
        }

        double get_sequence_length()
        {
            return tables->sequence_length;
        }
};


int
main()
{
    NodeTable nodes(nullptr);
    nodes.add_row(0, 1.0);
    nodes.add_row(0, 2.0);
    std::cout << "Straight table: num_rows = " << nodes.get_num_rows() << endl;

    TableCollection tables(10);
    std::cout << "Sequence length = " << tables.get_sequence_length() << endl;
    tables.nodes.add_row(0, 1.0);
    tables.nodes.add_row(0, 2.0);
    tables.nodes.add_row(0, 3.0);
    std::cout << "Via table collection: num_rows = " << tables.nodes.get_num_rows() << endl;

    return 0;

}
