//#include "test_BST.h"
//#include "lock_free_BST.h"
#include <getopt.h>
#include <string>

#include "benchmark.h"


void usage(const char *progname);

int processArgs(int argc, char **argv, int *mode, int *thread_num, int *size);

int main(int argc, char **argv) {
    int init_tree_size = 0;
    int mode = 0;
    int thread_num = 1;

    // parse arguments
    if (processArgs(argc, argv, &mode, &thread_num, &init_tree_size) == 1) {
        return 0;
    }

    Benchmark* benchmark = new Benchmark(100);
    switch (mode) {
        case 1:  // Fine-grained
            benchmark->run_fine_grained_BST(init_tree_size, thread_num);
            break;
        case 2:  // Coarse-grained
            benchmark->run_coarse_grained_BST(init_tree_size, thread_num);
            break;
        case 3:  // Lock-free
            benchmark->run_lock_free_BST(init_tree_size, thread_num);
            break;
        default:
            return 0;
    }

    delete benchmark;
    

//    TestBST* testor = new TestBST();
//    testor->test_coarse_grained_BST(4, 4);
//    testor->test_fine_grained_BST(4, 4);
//    testor->test_lock_free_BST(4, 4);
//    delete testor;

//    LockFreeBST* bst = new LockFreeBST();
//    bst->search(1);
//    bst->search(2);
//    bst->insert(1);
//    bst->insert(3);
//    bst->insert(2);
//    bst->insert(2);
//    bst->insert(2);
//    bst->search(1);
    return 0;
}

void usage(const char *progname) {
    cout << "Usage: " << progname << " [option] " << endl;
    cout << "Program options: " << endl;
    cout << "  -m  --mode <MODENAME>       BST mode: fine-grained, coarse-grained, lock-free" << endl;
    cout << "  -t  --thread <THREADNUM>    Number of thread to execute" << endl;
    cout << "  -s  --size <SIZE>           Initial BST size" << endl;
    cout << "  -h  --help                  This message"  << endl;
}

int processArgs(int argc, char **argv, int *mode, int *thread_num, int *size) {
    int opt;
    string mode_name;

    if (argc != 2 && argc != 7) {
        usage(argv[0]);
        return 1;
    }

    static struct option long_options[] = {
            {"help",    0, 0,  'h'},
            {"mode",    1, 0,  'm'},
            {"thread",  1, 0,  't'},
            {"size",    1, 0,  's'},
            {0 ,0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "m:t:s:h", long_options, NULL)) != EOF) {
        switch (opt) {
            case 'm':
                mode_name = std::string(optarg);
                if (mode_name == "fine-grained") {
                    *mode = 1;
                } else if (mode_name == "coarse-grained") {
                    *mode = 2;
                } else if (mode_name == "lock-free") {
                    *mode = 3;
                } else {
                    usage(argv[0]);
                    return 1;
                }
                break;
            case 't':
                *thread_num = std::stoi(optarg);
                break;
            case 's':
                *size = std::stoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                return 1;
            default:
                usage(argv[0]);
                return 1;
        }
    }
    return 0;
}