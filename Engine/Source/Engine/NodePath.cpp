#include "NodePath.h"
#include "Nodes/Node.h"
#include "Property.h"
#include "Script.h"

std::string FindRelativeNodePath(Node* src, Node* dst)
{
    if (src == nullptr || dst == nullptr)
        return "";

    if (src == dst)
        return ".";

    std::string path;

    std::vector<Node*> dstAncestors;

    // Find all of the dst node's ancestors
    {
        Node* dstAncestor = dst;

        while (dstAncestor != nullptr)
        {
            dstAncestors.push_back(dstAncestor);
            dstAncestor = dstAncestor->GetParent();
        }
    }

    // Find common ancestor between src and dst
    Node* commonAncestor = src;
    int32_t commonIndex = -1;

    while (commonAncestor != nullptr)
    {
        for (int32_t i = 0; i < int32_t(dstAncestors.size()); ++i)
        {
            if (commonAncestor == dstAncestors[i])
            {
                // Found the first common ancestor
                commonIndex = i;
                break;
            }
        }

        if (commonIndex != -1)
            break;

        path += "../";
        commonAncestor = commonAncestor->GetParent();
    }

    // No valid path
    if (commonAncestor == nullptr)
        return "";

    OCT_ASSERT(commonIndex >= 0);

    // Go backwards and append dst ancestor nodes onto the path until reaching the dst node
    int32_t dstIndex = commonIndex - 1;
    while (dstIndex >= 0)
    {
        path += dstAncestors[dstIndex]->GetName();

        if (dstIndex > 0)
        {
            path += "/";
        }

        --dstIndex;
    }

    return path;
}

void ResolveNodePaths(Node* node, bool recurseChildren)
{
    if (node == nullptr)
        return;

    // TODO: Perf optimization?
    //if (node->HasNodePathProperties())

    std::vector<Property> props;
    node->GatherProperties(props);

    for (uint32_t p = 0; p < props.size(); ++p)
    {
        if (props[p].mType == DatumType::Node &&
            props[p].mExtra != nullptr)
        {
            Property& prop = props[p];

            for (uint32_t i = 0; i < prop.GetCount(); ++i)
            {
                std::string path = prop.mExtra->GetString(i);
                Node* dst = ResolveNodePath(node, path);
                prop.SetNode(ResolveWeakPtr<Node>(dst), i);
            }

            //prop.DestroyExtraData();
        }
    }
}

Node* ResolveNodePath(Node* src, const std::string& path)
{
    if (src == nullptr)
        return nullptr;

    std::vector<std::string> tokens;

    int32_t start = 0;
    for (int32_t i = 0; i < path.size(); ++i)
    {
        if (path[i] == '/')
        {
            // Add a token
            if (start != i)
            {
                std::string token;
                token = path.substr(start, i - start);
                tokens.push_back(token);

                start = i + 1;
            }
        }
        else if (i == int32_t(path.size()) - 1)
        {
            if (start != i)
            {
                tokens.push_back(path.substr(start));
            }
        }
    }

    Node* dst = src;

    for (uint32_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i] == ".")
        {
            // Do nothing
        }
        else if (tokens[i] == "..")
        {
            if (dst->GetParent() != nullptr)
            {
                dst = dst->GetParent();
            }
        }
        else
        {
            const std::string& childName = tokens[i];
            dst = dst->FindChild(childName, false);
        }

        // Failed to follow the chain
        if (dst == nullptr)
        {
            break;
        }
    }

    return dst;
}

void ResolvePendingNodePaths(std::vector<PendingNodePath>& pending)
{
    for (uint32_t i = 0; i < pending.size(); ++i)
    {
        Node* node = pending[i].mNode.Get();
        const std::string& propName = pending[i].mPropName;
        const Datum& path = pending[i].mPath;

        if (node)
        {
            std::vector<Property> props;
            node->GatherProperties(props);

            for (uint32_t p = 0; p < props.size(); ++p)
            {
                Property& prop = props[p];
                if (prop.mName != propName)
                    continue;

                if (prop.mType == DatumType::Node &&
                    prop.mCount == path.mCount)
                {
                    for (uint32_t n = 0; n < prop.mCount; ++n)
                    {
                        const std::string& nPath = path.GetString(n);
                        if (nPath != "")
                        {
                            Node* targetNode = ResolveNodePath(node, nPath);
                            prop.SetNode(ResolveWeakPtr<Node>(targetNode), n);
                        }
                    }
                }
                else
                {
                    LogWarning("Failed to resolve node path. Type or count mismatch.");
                }
            }
        }
    }
}

void ResolveAllNodePathsRecursive(Node* node)
{
    auto resolvePaths = [](Node* node, std::vector<Property>& props)
    {
        for (uint32_t p = 0; p < props.size(); ++p)
        {
            Property& prop = props[p];
            if (prop.mType == DatumType::Node &&
                prop.mExtra != nullptr)
            {
                for (uint32_t c = 0; c < prop.GetCount(); ++c)
                {
                    Node* targetNode = ResolveNodePath(node, prop.mExtra->GetString(c));
                    prop.SetNode(ResolveWeakPtr(targetNode), c);
                }
            }
        }
    };

    std::vector<Property> props;
    node->GatherProperties(props);
    resolvePaths(node, props);

    if (node->GetScript())
    {
        std::vector<Property>& scriptProps = node->GetScript()->GetScriptProperties();
        resolvePaths(node, scriptProps);
    }

    for (uint32_t c = 0; c < node->GetNumChildren(); ++c)
    {
        ResolveAllNodePathsRecursive(node->GetChild(c));
    }
}

void RecordNodePaths(Node* node, std::vector<Property>& props)
{
    // Find node paths for Node properties
    for (uint32_t i = 0; i < props.size(); ++i)
    {
        Property& prop = props[i];
        if (prop.mType == DatumType::Node)
        {
            prop.CreateExtraData();
            prop.mExtra->Destroy();

            for (uint32_t c = 0; c < prop.mCount; ++c)
            {
                const WeakPtr<Node>& dstNode = prop.mData.n[c];
                std::string nodePath = FindRelativeNodePath(node, dstNode.Get());
                prop.mExtra->PushBack(nodePath);
            }
        }
    }
}
