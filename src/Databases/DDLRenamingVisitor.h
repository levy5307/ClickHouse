#pragma once

#include <Core/Types.h>
#include <Core/QualifiedTableName.h>
#include <Interpreters/InDepthNodeVisitor.h>
#include <memory>
#include <unordered_map>


namespace DB
{
class IAST;
using ASTPtr = std::shared_ptr<IAST>;
class Context;
using ContextPtr = std::shared_ptr<const Context>;
class DDLRenamingMap;

/// Changes names of databases or tables in a create query according to a specified renaming map.
/// Does not validate AST, works a best-effort way.
void renameDatabaseAndTableNameInCreateQuery(const ContextPtr & global_context, const DDLRenamingMap & renaming_map, ASTPtr & ast);

/// Renaming map keeps information about new names of databases or tables.
class DDLRenamingMap
{
public:
    void setNewTableName(const QualifiedTableName & old_table_name, const QualifiedTableName & new_table_name);
    void setNewDatabaseName(const String & old_database_name, const String & new_database_name);
    void setNewTemporaryTableName(const String & old_table_name, const String & new_table_name);

    QualifiedTableName getNewTableName(const QualifiedTableName & old_table_name) const;
    const String & getNewDatabaseName(const String & old_database_name) const;
    const String & getNewTemporaryTableName(const String & old_table_name) const;

private:
    std::unordered_map<QualifiedTableName, QualifiedTableName> old_to_new_table_names;
    std::unordered_map<String, String> old_to_new_database_names;
    std::unordered_map<String, String> old_to_new_temporary_table_names;
};

/// Visits ASTCreateQuery and changes names of databases or tables.
class DDLRenamingVisitor
{
public:
    struct Data
    {
        ContextPtr global_context;
        const DDLRenamingMap & renaming_map;
        ASTPtr create_query;
    };

    using Visitor = InDepthNodeVisitor<DDLRenamingVisitor, false>;

    static bool needChildVisit(const ASTPtr &, const ASTPtr &);
    static void visit(ASTPtr ast, const Data & data);
};

}
