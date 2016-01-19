#pragma once

namespace atom {
class NodeBindings;
}

namespace node {
class Environment;
}

namespace silkedit_node {

void Start(int argc, char** argv, atom::NodeBindings *nodeBindings);
void Cleanup(node::Environment *env);

}  // namespace silkedit_node

