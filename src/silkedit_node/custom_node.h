#include <node.h>

#include "atom/node_bindings.h"

namespace silkedit_node {

void Start(int argc, char** argv, atom::NodeBindings *nodeBindings);
void Cleanup(node::Environment *env);

}  // namespace silkedit_node

