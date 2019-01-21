/* C++ wrapper experiement. */

#include <iostream>
#include <stdexcept>
#include <sstream>

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
        bool malloced_locally = true;

    public:
        NodeTable()
        {
            table = (tsk_node_table_t *) malloc(sizeof(*table));
            if (table == NULL) {
                throw std::runtime_error("Out of memory");
            }
            malloced_locally = true;
            int ret = tsk_node_table_init(table, 0);
            check_error(ret);
        }

        NodeTable(tsk_node_table_t *the_table)
        {
            /* FIXME: leaking memory here because the standard constructor has already been
             * called. Must be a way to dealing with this though. */
            table = the_table;
            std::cout << "table constructor" << endl;
            malloced_locally = false;
        }

        ~NodeTable()
        {
            if (malloced_locally && table != NULL) {
                tsk_node_table_free(table);
                free(table);
            }
        }


        tsk_id_t add_row(tsk_flags_t flags, double time) /* and more params */
        {
            tsk_id_t ret = tsk_node_table_add_row(table, flags, time, TSK_NULL,
                    TSK_NULL, NULL, 0);
            check_error(ret);
            return ret;
        }

        tsk_size_t get_num_rows(void)
        {
            return table->num_rows;
        }
        /* Etc */
};

class TableCollection
{

    private:
        tsk_table_collection_t *tables;

    public:
        NodeTable nodes;

        TableCollection(double sequence_length)
        {
            tables = (tsk_table_collection_t *) malloc(sizeof(*tables));
            if (tables == NULL) {
                throw std::runtime_error("Out of memory");
            }
            int ret = tsk_table_collection_init(tables, 0);
            check_error(ret);
            tables->sequence_length = sequence_length;
            nodes = NodeTable(&tables->nodes);
        }

        ~TableCollection()
        {
            if (tables != NULL) {
                tsk_table_collection_free(tables);
                free(tables);
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
    NodeTable nodes;

    nodes.add_row(0, 1.0);
    nodes.add_row(0, 2.0);
    std::cout << "Straight table: num_rows = " << nodes.get_num_rows() << endl;

    TableCollection tables = TableCollection(10);
    std::cout << "Sequence length = " << tables.get_sequence_length() << endl;
    tables.nodes.add_row(0, 1.0);
    tables.nodes.add_row(0, 2.0);
    tables.nodes.add_row(0, 3.0);
    std::cout << "Via table collection: num_rows = " << tables.nodes.get_num_rows() << endl;

    return 0;

}
