/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef FRONTENDS_P4_SIMPLIFY_H_
#define FRONTENDS_P4_SIMPLIFY_H_

#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4/methodInstance.h"
#include "frontends/p4/typeChecking/typeChecker.h"
#include "ir/ir.h"

namespace P4 {

/** @brief Replace complex control flow nodes with simpler ones where possible.
 *
 * Simplify the IR in the following ways:
 *
 * 1. Remove empty statements from within parser states, actions, and block
 * statements.
 *
 * 2. Replace if statements with empty bodies with an empty statement.
 *
 * 3. Remove fallthrough switch cases that are not followed by a case with a
 * statement.
 *
 * 4. Replace switch statements that switch on a table application but have no
 * cases with a table application expression.
 *
 * 5. If a block statement is within another block or a parser state, and (a)
 * is empty, then replace it with an empty statement, or (b) does not contain
 * declarations, then move its component statements to the enclosing block.
 *
 * 6. If a block statement in an if statement branch only contains a single
 * statement, replace the block with the statement it contains.
 *
 * @pre An up-to-date ReferenceMap and TypeMap.
 */
class DoSimplifyControlFlow : public Transform, public ResolutionContext {
    TypeMap *typeMap;
    bool foldInlinedFrom;

 public:
    explicit DoSimplifyControlFlow(TypeMap *typeMap, bool foldInlinedFrom)
        : typeMap(typeMap), foldInlinedFrom(foldInlinedFrom) {
        CHECK_NULL(typeMap);
        setName("DoSimplifyControlFlow");
        // We may want to replace the same statement with different things
        // in different places.
        visitDagOnce = false;
    }
    const IR::Node *postorder(IR::BlockStatement *statement) override;
    const IR::Node *postorder(IR::IfStatement *statement) override;
    const IR::Node *postorder(IR::EmptyStatement *statement) override;
    const IR::Node *postorder(IR::SwitchStatement *statement) override;
};

/// Repeatedly simplify control flow until convergence, as some simplification
/// steps enable further simplification.
class SimplifyControlFlow : public PassRepeated {
 public:
    explicit SimplifyControlFlow(TypeMap *typeMap, bool foldInlinedFrom,
                                 TypeChecking *typeChecking = nullptr) {
        if (!typeChecking) typeChecking = new TypeChecking(nullptr, typeMap);
        passes.push_back(typeChecking);
        passes.push_back(new DoSimplifyControlFlow(typeMap, foldInlinedFrom));
        setName("SimplifyControlFlow");
    }
};

}  // namespace P4

#endif /* FRONTENDS_P4_SIMPLIFY_H_ */
