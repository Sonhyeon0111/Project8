// Problem8View.cpp: CProblem8View 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Problem8.h"
#endif

#include "Problem8Doc.h"
#include "Problem8View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProblem8View

IMPLEMENT_DYNCREATE(CProblem8View, CView)

BEGIN_MESSAGE_MAP(CProblem8View, CView)
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CProblem8View 생성/소멸

CProblem8View::CProblem8View() noexcept
{
    m_start = -1;
    m_end = -1;
}

CProblem8View::~CProblem8View()
{
}

BOOL CProblem8View::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}


// CProblem8View 그리기

void CProblem8View::OnDraw(CDC* pDC)
{
    CProblem8Doc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    // 1) 배경 이미지 그리기 (CImage)
    if (!m_bg.IsNull())
        m_bg.Draw(pDC->m_hDC, 0, 0);

    // 2) Edge 그리기
    for (auto& e : m_edges)
    {
        CPen pen(PS_SOLID, e.inPath ? 3 : 1,
            e.inPath ? RGB(255, 0, 0) : RGB(0, 0, 0));
        CPen* oldPen = pDC->SelectObject(&pen);

        pDC->MoveTo(m_nodes[e.from].pos);
        pDC->LineTo(m_nodes[e.to].pos);

        pDC->SelectObject(oldPen);
    }

    // 3) Node 그리기
    for (auto& n : m_nodes)
    {
        CBrush brush(n.inPath ? RGB(255, 0, 0) : RGB(0, 0, 255));
        CBrush* oldBrush = pDC->SelectObject(&brush);

        pDC->Ellipse(n.pos.x - 5, n.pos.y - 5,
            n.pos.x + 5, n.pos.y + 5);

        pDC->SelectObject(oldBrush);
    }
}


// CProblem8View 진단

#ifdef _DEBUG
void CProblem8View::AssertValid() const
{
    CView::AssertValid();
}

void CProblem8View::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CProblem8Doc* CProblem8View::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProblem8Doc)));
    return (CProblem8Doc*)m_pDocument;
}
#endif


// 초기화 시 이미지 로드

void CProblem8View::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    // 폴더에 map.png 를 두고 상대 경로로 로드
    if (m_bg.IsNull())
        m_bg.Load(_T("map.png"));
}


// ===== 보조 함수들 =====

int CProblem8View::FindNode(CPoint p)
{
    const int R = 10;
    for (int i = 0; i < (int)m_nodes.size(); ++i)
    {
        int dx = p.x - m_nodes[i].pos.x;
        int dy = p.y - m_nodes[i].pos.y;
        if (dx * dx + dy * dy <= R * R)
            return i;
    }
    return -1;
}

void CProblem8View::AddEdge(int a, int b)
{
    Edge e;
    e.from = a;
    e.to = b;
    e.weight = Distance(m_nodes[a].pos, m_nodes[b].pos);
    m_edges.push_back(e);
}

void CProblem8View::ClearPath()
{
    for (auto& n : m_nodes)
        n.inPath = false;
    for (auto& e : m_edges)
        e.inPath = false;
}

double CProblem8View::Distance(CPoint a, CPoint b)
{
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return std::sqrt((double)(dx * dx + dy * dy));
}

void CProblem8View::RunDijkstra(int start, int goal)
{
    int n = (int)m_nodes.size();
    const double INF = 1e18;

    std::vector<double> dist(n, INF);
    std::vector<int>    prev(n, -1);
    std::vector<bool>   used(n, false);
    std::vector<std::vector<std::pair<int, double>>> adj(n);

    for (auto& e : m_edges)
    {
        adj[e.from].push_back({ e.to, e.weight });
        adj[e.to].push_back({ e.from, e.weight });
    }

    dist[start] = 0.0;

    for (int i = 0; i < n; ++i)
    {
        int u = -1;
        double best = INF;

        for (int v = 0; v < n; ++v)
            if (!used[v] && dist[v] < best)
            {
                best = dist[v];
                u = v;
            }

        if (u == -1) break;
        used[u] = true;
        if (u == goal) break;

        for (auto& pr : adj[u])
        {
            int v = pr.first;
            double w = pr.second;
            if (dist[v] > dist[u] + w)
            {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
        }
    }

    if (dist[goal] >= INF)
    {
        AfxMessageBox(_T("연결된 경로가 없습니다."));
        return;
    }

    ClearPath();

    int cur = goal;
    while (cur != -1)
    {
        m_nodes[cur].inPath = true;
        cur = prev[cur];
    }

    for (auto& e : m_edges)
        if (m_nodes[e.from].inPath && m_nodes[e.to].inPath)
            e.inPath = true;

    CString msg;
    msg.Format(_T("최단 거리: %.2f 픽셀"), dist[goal]);
    AfxMessageBox(msg);
}


// CProblem8View 메시지 처리기

void CProblem8View::OnLButtonDown(UINT nFlags, CPoint point)
{
    BOOL bCtrl = (nFlags & MK_CONTROL) != 0;
    BOOL bShift = (nFlags & MK_SHIFT) != 0;
    BOOL bAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;

    // 1) Ctrl → 점 생성
    if (bCtrl && !bShift && !bAlt)
    {
        Node nd;
        nd.pos = point;
        m_nodes.push_back(nd);
        Invalidate();
        return;
    }

    // 2) Alt → 점 선택 후 Edge 생성
    if (bAlt && !bCtrl && !bShift)
    {
        int idx = FindNode(point);
        if (idx != -1)
        {
            m_nodes[idx].selected = true;

            std::vector<int> sel;
            for (int i = 0; i < (int)m_nodes.size(); ++i)
                if (m_nodes[i].selected)
                    sel.push_back(i);

            if (sel.size() == 2)
            {
                AddEdge(sel[0], sel[1]);
                m_nodes[sel[0]].selected = false;
                m_nodes[sel[1]].selected = false;
            }
            Invalidate();
        }
        return;
    }

    // 3) Ctrl + Shift → 최단거리 계산
    if (bCtrl && bShift && !bAlt)
    {
        int idx = FindNode(point);
        if (idx != -1)
        {
            if (m_start == -1)
                m_start = idx;
            else if (m_end == -1)
            {
                m_end = idx;
                RunDijkstra(m_start, m_end);
            }
            else
            {
                m_start = idx;
                m_end = -1;
                ClearPath();
            }
            Invalidate();
        }
        return;
    }

    CView::OnLButtonDown(nFlags, point);
}

