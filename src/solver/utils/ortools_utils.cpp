#include "ortools_utils.h"

#include <antares/logs.h>
#include <antares/study.h>
#include <antares/exception/AssertionError.hpp>
#include <antares/Enum.hpp>

#include <yuni/core/system/memory.h>

using namespace operations_research;

template<typename T_PROBLEM>
MPSolver* convert_to_MPSolver(T_PROBLEM* problemeSimplexe);

void extract_from_MPSolver(MPSolver* solver, PROBLEME_SIMPLEXE* problemeSimplexe);
void extract_from_MPSolver(MPSolver* solver, PROBLEME_A_RESOUDRE* problemeSimplexe);

void change_MPSolver_objective(MPSolver* solver, double* costs, int nbVar);
void change_MPSolver_rhs(MPSolver* solver, double* rhs, char* sens, int nbRow);
void change_MPSolver_variables_bounds(MPSolver* solver,
                                      double* bMin,
                                      double* bMax,
                                      int nbVar,
                                      PROBLEME_SIMPLEXE* problemeSimplexe);

void transferVariables(MPSolver* solver, double* bMin, double* bMax, double* costs, int nbVar)
{
    MPObjective* const objective = solver->MutableObjective();
    for (int idxVar = 0; idxVar < nbVar; ++idxVar)
    {
        std::ostringstream oss;
        oss << "x" << idxVar;
        double min_l = 0.0;
        if (bMin != NULL)
        {
            min_l = bMin[idxVar];
        }
        double max_l = bMax[idxVar];
        auto x = solver->MakeNumVar(min_l, max_l, oss.str());
        objective->SetCoefficient(x, costs[idxVar]);
    }
}

void transferRows(MPSolver* solver, double* rhs, char* sens, int nbRow)
{
    for (int idxRow = 0; idxRow < nbRow; ++idxRow)
    {
        double bMin = -MPSolver::infinity(), bMax = MPSolver::infinity();
        if (sens[idxRow] == '=')
        {
            bMin = bMax = rhs[idxRow];
        }
        else if (sens[idxRow] == '<')
        {
            bMax = rhs[idxRow];
        }
        else if (sens[idxRow] == '>')
        {
            bMin = rhs[idxRow];
        }
        std::ostringstream oss;
        oss << "c" << idxRow;
        MPConstraint* const ct = solver->MakeRowConstraint(bMin, bMax, oss.str());
    }
}

void transferMatrix(MPSolver* solver,
                    int* indexRows,
                    int* terms,
                    int* indexCols,
                    double* coeffs,
                    int nbRow)
{
    auto variables = solver->variables();
    auto constraints = solver->constraints();

    for (int idxRow = 0; idxRow < nbRow; ++idxRow)
    {
        MPConstraint* const ct = constraints[idxRow];
        int debutLigne = indexRows[idxRow];
        for (int idxCoef = 0; idxCoef < terms[idxRow]; ++idxCoef)
        {
            int pos = debutLigne + idxCoef;
            ct->SetCoefficient(variables[indexCols[pos]], coeffs[pos]);
        }
    }
}

MPSolver* convert_to_MPSolver(PROBLEME_SIMPLEXE* problemeSimplexe)
{
    auto& study = *Data::Study::Current::Get();

    // Define solver used depending on study option
    MPSolver::OptimizationProblemType solverType
      = OrtoolsUtils().getLinearOptimProblemType(study.parameters.ortoolsEnumUsed);

    // Create the linear solver instance
    MPSolver* solver = new MPSolver("simple_lp_program", solverType);

    // Create the variables and set objective cost.
    transferVariables(solver,
                      problemeSimplexe->Xmin,
                      problemeSimplexe->Xmax,
                      problemeSimplexe->CoutLineaire,
                      problemeSimplexe->NombreDeVariables);

    // Create constraints and set coefs
    transferRows(solver,
                 problemeSimplexe->SecondMembre,
                 problemeSimplexe->Sens,
                 problemeSimplexe->NombreDeContraintes);
    transferMatrix(solver,
                   problemeSimplexe->IndicesDebutDeLigne,
                   problemeSimplexe->NombreDeTermesDesLignes,
                   problemeSimplexe->IndicesColonnes,
                   problemeSimplexe->CoefficientsDeLaMatriceDesContraintes,
                   problemeSimplexe->NombreDeContraintes);

    return solver;
}

MPSolver* convert_to_MPSolver(PROBLEME_A_RESOUDRE* problemeAResoudre)
{
    auto& study = *Data::Study::Current::Get();

    // Define solver used depending on study option
    MPSolver::OptimizationProblemType solverType
      = OrtoolsUtils().getMixedIntegerOptimProblemType(study.parameters.ortoolsEnumUsed);

    MPSolver* solver = new MPSolver("simple_lp_program", solverType);

    // Create the variables and set objective cost.
    transferVariables(solver,
                      problemeAResoudre->Xmin,
                      problemeAResoudre->Xmax,
                      problemeAResoudre->CoutLineaire,
                      problemeAResoudre->NombreDeVariables);

    // Create constraints and set coefs
    transferRows(solver,
                 problemeAResoudre->SecondMembre,
                 problemeAResoudre->Sens,
                 problemeAResoudre->NombreDeContraintes);
    transferMatrix(solver,
                   problemeAResoudre->IndicesDebutDeLigne,
                   problemeAResoudre->NombreDeTermesDesLignes,
                   problemeAResoudre->IndicesColonnes,
                   problemeAResoudre->CoefficientsDeLaMatriceDesContraintes,
                   problemeAResoudre->NombreDeContraintes);

    return solver;
}

void extract_from_MPSolver(MPSolver* solver, PROBLEME_A_RESOUDRE* problemePne)
{
    auto& variables = solver->variables();
    int nbVar = problemePne->NombreDeVariables;

    // Extracting variable values and reduced costs
    for (int idxVar = 0; idxVar < nbVar; ++idxVar)
    {
        auto& var = variables[idxVar];
        problemePne->X[idxVar] = var->solution_value();
    }

    auto& constraints = solver->constraints();
    int nbRow = problemePne->NombreDeContraintes;
    for (int idxRow = 0; idxRow < nbRow; ++idxRow)
    {
        auto& row = constraints[idxRow];
        problemePne->VariablesDualesDesContraintes[idxRow] = row->dual_value();
    }
}

void extract_from_MPSolver(MPSolver* solver, PROBLEME_SIMPLEXE* problemeSimplexe)
{
    auto& variables = solver->variables();
    int nbVar = problemeSimplexe->NombreDeVariables;

    // Extracting variable values and reduced costs
    for (int idxVar = 0; idxVar < nbVar; ++idxVar)
    {
        auto& var = variables[idxVar];
        problemeSimplexe->X[idxVar] = var->solution_value();
        problemeSimplexe->CoutsReduits[idxVar] = var->reduced_cost();
    }

    auto& constraints = solver->constraints();
    int nbRow = problemeSimplexe->NombreDeContraintes;
    for (int idxRow = 0; idxRow < nbRow; ++idxRow)
    {
        auto& row = constraints[idxRow];
        problemeSimplexe->CoutsMarginauxDesContraintes[idxRow] = row->dual_value();
    }
}

void change_MPSolver_objective(MPSolver* solver, double* costs, int nbVar)
{
    auto& variables = solver->variables();
    for (int idxVar = 0; idxVar < nbVar; ++idxVar)
    {
        auto& var = variables[idxVar];
        solver->MutableObjective()->SetCoefficient(var, costs[idxVar]);
    }
}

void change_MPSolver_variables_bounds(MPSolver* solver,
                                      double* bMin,
                                      double* bMax,
                                      int nbVar,
                                      PROBLEME_SIMPLEXE* problemeSimplexe)
{
    auto& variables = solver->variables();
    for (int idxVar = 0; idxVar < nbVar; ++idxVar)
    {
        double min_l
          = ((problemeSimplexe->TypeDeVariable[idxVar] == VARIABLE_NON_BORNEE)
                 || (problemeSimplexe->TypeDeVariable[idxVar] == VARIABLE_BORNEE_SUPERIEUREMENT)
               ? -MPSolver::infinity()
               : bMin[idxVar]);
        double max_l
          = ((problemeSimplexe->TypeDeVariable[idxVar] == VARIABLE_NON_BORNEE)
                 || (problemeSimplexe->TypeDeVariable[idxVar] == VARIABLE_BORNEE_INFERIEUREMENT)
               ? MPSolver::infinity()
               : bMax[idxVar]);
        auto& var = variables[idxVar];
        var->SetBounds(min_l, max_l);
    }
}

void change_MPSolver_rhs(MPSolver* solver, double* rhs, char* sens, int nbRow)
{
    auto& constraints = solver->constraints();
    for (int idxRow = 0; idxRow < nbRow; ++idxRow)
    {
        if (sens[idxRow] == '=')
            constraints[idxRow]->SetBounds(rhs[idxRow], rhs[idxRow]);
        else if (sens[idxRow] == '<')
            constraints[idxRow]->SetBounds(-MPSolver::infinity(), rhs[idxRow]);
        else if (sens[idxRow] == '>')
            constraints[idxRow]->SetBounds(rhs[idxRow], MPSolver::infinity());
    }
}

void ORTOOLS_EcrireJeuDeDonneesLineaireAuFormatMPS(MPSolver* solver, size_t numSpace, int const n)
{
    auto& study = *Antares::Data::Study::Current::Get();

    int const year = study.runtime->currentYear[numSpace] + 1;
    int const week = study.runtime->weekInTheYear[numSpace] + 1;
    std::stringstream buffer;
    buffer << study.folderOutput << Yuni::IO::Separator << "problem-" << year << "-" << week << "-"
           << n << ".mps";
    solver->Write(buffer.str());
}

std::string getRunName(std::string const& prefix, size_t numSpace, int numInterval, int numOptim)
{
    auto& study = *Antares::Data::Study::Current::Get();

    int const year = study.runtime->currentYear[numSpace] + 1;
    int const week = study.runtime->weekInTheYear[numSpace] + 1;
    std::stringstream buffer;
    buffer << prefix << " for year=" << year << ", week=" << week << ", interval=" << numInterval
           << ", optimisation=" << numOptim;
    return buffer.str();
}

bool solveAndManageStatus(MPSolver* solver, int& resultStatus, MPSolverParameters& params)
{
    auto status = solver->Solve(params);

    if (status == MPSolver::OPTIMAL || status == MPSolver::FEASIBLE)
    {
        resultStatus = OUI_SPX;
    }
    else
    {
        resultStatus = NON_SPX;
    }

    return resultStatus == OUI_SPX;
}

MPSolver* solveProblem(PROBLEME_SIMPLEXE* Probleme, MPSolver* ProbSpx)
{
    MPSolver* solver = ProbSpx;

    if (solver == NULL)
    {
        solver = convert_to_MPSolver(Probleme);
    }

    MPSolverParameters params;

    if (solveAndManageStatus(solver, Probleme->ExistenceDUneSolution, params))
    {
        extract_from_MPSolver(solver, Probleme);
    }

    return solver;
}

MPSolver* solveProblem(PROBLEME_A_RESOUDRE* Probleme, MPSolver* ProbSpx)
{
    MPSolver* solver = ProbSpx;

    if (solver == NULL)
    {
        solver = convert_to_MPSolver(Probleme);
    }

    MPSolverParameters params;
    if (Probleme->FaireDuPresolve == NON_PNE)
    {
        params.SetIntegerParam(MPSolverParameters::PRESOLVE, MPSolverParameters::PRESOLVE_OFF);
    }
    else
    {
        params.SetIntegerParam(MPSolverParameters::PRESOLVE, MPSolverParameters::PRESOLVE_ON);
    }

    if (solveAndManageStatus(solver, Probleme->ExistenceDUneSolution, params))
    {
        extract_from_MPSolver(solver, Probleme);
    }

    return solver;
}

extern "C"
{
    MPSolver* ORTOOLS_Simplexe(PROBLEME_SIMPLEXE* Probleme, MPSolver* ProbSpx)
    {
        return solveProblem(Probleme, ProbSpx);
    }

    MPSolver* ORTOOLS_Simplexe_PNE(PROBLEME_A_RESOUDRE* Probleme, MPSolver* ProbSpx)
    {
        return solveProblem(Probleme, ProbSpx);
    }

    void ORTOOLS_ModifierLeVecteurCouts(MPSolver* solver, double* costs, int nbVar)
    {
        change_MPSolver_objective(solver, costs, nbVar);
    }

    void ORTOOLS_ModifierLeVecteurSecondMembre(MPSolver* solver, double* rhs, char* sens, int nbRow)
    {
        change_MPSolver_rhs(solver, rhs, sens, nbRow);
    }

    void ORTOOLS_CorrigerLesBornes(MPSolver* solver,
                                   double* bMin,
                                   double* bMax,
                                   int* typeVar,
                                   int nbVar,
                                   PROBLEME_SIMPLEXE* Probleme)
    {
        Probleme->TypeDeVariable = typeVar;
        change_MPSolver_variables_bounds(solver, bMin, bMax, nbVar, Probleme);
    }

    void ORTOOLS_LibererProbleme(MPSolver* solver)
    {
        delete solver;
    }
}

using namespace Antares::Data;

OrtoolsUtils::OrtoolsUtils()
{
    // TODO JMK : Values must be adequacy with kOptimizationProblemTypeNames for ortools
    // linear_solver/linear_solver.cc file

    _solverLinearProblemOptimStringMap[OrtoolsSolver::sirius]
      = "sirius_lp"; // TODO JMK : not defined in current ortools RTE branch.
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::sirius]
      = "sirius_mip"; // TODO JMK : not defined in current ortools RTE branch.

    _solverLinearProblemOptimStringMap[OrtoolsSolver::coin] = "clp";
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::coin] = "cbc";

    _solverLinearProblemOptimStringMap[OrtoolsSolver::xpress] = "xpress_lp";
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::xpress] = "xpress_mip";

    _solverLinearProblemOptimStringMap[OrtoolsSolver::glop_scip] = "glop";
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::glop_scip] = "scip";

    _solverLinearProblemOptimStringMap[OrtoolsSolver::cplex]
      = "cplex_lp"; // TODO JMK : not defined in current ortools RTE branch.
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::cplex]
      = "cplex_mip"; // TODO JMK : not defined in current ortools RTE branch.

    _solverLinearProblemOptimStringMap[OrtoolsSolver::gurobi] = "gurobi_lp";
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::gurobi] = "gurobi_mip";

    _solverLinearProblemOptimStringMap[OrtoolsSolver::glpk] = "glpk_lp";
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::glpk] = "glpk_mip";

    _solverLinearProblemOptimStringMap[OrtoolsSolver::glop_cbc] = "glop";
    _solverMixedIntegerProblemOptimStringMap[OrtoolsSolver::glop_cbc] = "cbc";

    /* TODO JMK : see how we can get optimization problem type with current ortools RTE branch
    (can't use enum because of compile switch)

    _solverLinearProblemOptimMap[OrtoolsSolver::sirius]       =
    MPSolver::OptimizationProblemType::SIRIUS_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::sirius] =
    MPSolver::OptimizationProblemType::SIRIUS_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::coin]       =
    MPSolver::OptimizationProblemType::CLP_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::coin] =
    MPSolver::OptimizationProblemType::CBC_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::xpress]       =
    MPSolver::OptimizationProblemType::XPRESS_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::xpress] =
    MPSolver::OptimizationProblemType::XPRESS_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::glop_scip]       =
    MPSolver::OptimizationProblemType::GLOP_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::glop_scip] =
    MPSolver::OptimizationProblemType::SCIP_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::cplex]       =
    MPSolver::OptimizationProblemType::CPLEX_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::cplex] =
    MPSolver::OptimizationProblemType::CPLEX_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::gurobi]       =
    MPSolver::OptimizationProblemType::GUROBI_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::gurobi] =
    MPSolver::OptimizationProblemType::GUROBI_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::glpk]       =
    MPSolver::OptimizationProblemType::GLPK_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::glpk] =
    MPSolver::OptimizationProblemType::GLPK_MIXED_INTEGER_PROGRAMMING;

    _solverLinearProblemOptimMap[OrtoolsSolver::glop_cbc]       =
    MPSolver::OptimizationProblemType::GLOP_LINEAR_PROGRAMMING;
    _solverMixedIntegerProblemOptimMap[OrtoolsSolver::glop_cbc] =
    MPSolver::OptimizationProblemType::CBC_MIXED_INTEGER_PROGRAMMING;

            */
}

bool OrtoolsUtils::isOrtoolsSolverAvailable(OrtoolsSolver ortoolsSolver)
{
    bool result = false;

    // GLOP solver fail for too many examples. For now support is disabled
    if (ortoolsSolver == OrtoolsSolver::glop_scip || ortoolsSolver == OrtoolsSolver::glop_cbc)
    {
        return false;
    }

    try
    {
        result = MPSolver::SupportsProblemType(getLinearOptimProblemType(ortoolsSolver));
        result &= MPSolver::SupportsProblemType(getMixedIntegerOptimProblemType(ortoolsSolver));
    }
    catch (AssertionError& ex)
    {
        result = false;
    }

    return result;
}

std::list<OrtoolsSolver> OrtoolsUtils::getAvailableOrtoolsSolver()
{
    std::list<OrtoolsSolver> result;

    std::list<OrtoolsSolver> solverList = Enum::enumList<Antares::Data::OrtoolsSolver>();

    for (OrtoolsSolver solver : solverList)
    {
        if (isOrtoolsSolverAvailable(solver))
        {
            result.push_back(solver);
        }
    }

    return result;
}

std::list<std::string> OrtoolsUtils::getAvailableOrtoolsSolverName()
{
    std::list<std::string> result;

    std::list<OrtoolsSolver> solverList = getAvailableOrtoolsSolver();

    for (OrtoolsSolver solver : solverList)
    {
        result.push_back(Enum::toString(solver));
    }

    return result;
}

MPSolver::OptimizationProblemType OrtoolsUtils::getLinearOptimProblemType(
  const OrtoolsSolver& ortoolsSolver)
{
    // TODO JMK : see how we can get optimization problem type with current ortools RTE branch
    // (can't use enum because of compile switch) MPSolver::OptimizationProblemType result =
    // _solverLinearProblemOptimMap.at(ortoolsSolver)

    // For now we are using string to get optimization problem type
    MPSolver::OptimizationProblemType result
      = MPSolver::OptimizationProblemType::GLOP_LINEAR_PROGRAMMING;

    if (!MPSolver::ParseSolverType(_solverLinearProblemOptimStringMap.at(ortoolsSolver), &result))
    {
        throw AssertionError("Unsupported Linear OrtoolsSolver for solver "
                             + Enum::toString(ortoolsSolver));
    }

    return result;
}

MPSolver::OptimizationProblemType OrtoolsUtils::getMixedIntegerOptimProblemType(
  const OrtoolsSolver& ortoolsSolver)
{
    // TODO JMK : see how we can get optimization problem type with current ortools RTE branch
    // (can't use enum because of compile switch) MPSolver::OptimizationProblemType result =
    // _solverMixedIntegerProblemOptimMap.at(ortoolsSolver)

    // For now we are using string to get optimization problem type
    MPSolver::OptimizationProblemType result
      = MPSolver::OptimizationProblemType::GLOP_LINEAR_PROGRAMMING;

    if (!MPSolver::ParseSolverType(_solverMixedIntegerProblemOptimStringMap[ortoolsSolver],
                                   &result))
    {
        throw AssertionError("Unsupported Mixed Integer OrtoolsSolver for solver "
                             + Enum::toString(ortoolsSolver));
    }

    return result;
}