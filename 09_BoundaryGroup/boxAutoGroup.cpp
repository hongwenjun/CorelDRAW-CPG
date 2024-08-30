#include "cdrapp.h"

// 扩展边界框
void expand_bounding_boxes(std::vector<BoundingBox>& boxes, double exp) {
    for (auto& box : boxes) {
        // 扩展宽度和高度
        box.w += 2 * exp; // 左右各扩展
        box.h += 2 * exp; // 上下各扩展
        // 调整左下角坐标
        box.x -= exp; // 向左扩展
        box.y -= exp; // 向下扩展
    }
}

double get_bounding_box_area(BoundingBox box) { return box.w * box.h; }

// 自定义比较函数，按 BoundingBox 的面积大小进行排序
bool compare_bounding_boxes(const std::pair<int, BoundingBox> &a,
                            const std::pair<int, BoundingBox> &b) {
  return get_bounding_box_area(a.second) > get_bounding_box_area(b.second);
}

// 查找父节点
int find(std::vector<int>& parent, int x) {
    if (parent[x] != x) {
        parent[x] = find(parent, parent[x]);
    }
    return parent[x];
}

// 合并两个集合
void unionSet(std::vector<int>& parent, int x, int y) {
    int xroot = find(parent, x);
    int yroot = find(parent, y);
    parent[xroot] = yroot;
}
// 检查两个矩形是否重叠
bool isOverlapped(const BoundingBox &a, const BoundingBox &b) {
  return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
} // 函数使用AABB（Axis-Aligned Bounding Box）碰撞检测

bool BBox_DrawRectangle(corel *cdr, double exp) {
  BoundingBox box;
  auto sr = cdr->ActiveSelectionRange; // 获得选择范围
  auto al = cdr->ActiveLayer;          // 获得当前层
  if (!sr || !al) return false;
  
  BeginOpt(cdr);
  auto srs = cdr->CreateShapeRange();

  // CorelDRAW Shapes 物件 Item 编号从1开始
  for (auto i = 0; i != sr->Count; i++) {
    sr->Shapes->Item[i + 1]->GET_BOUNDING_BOX(box); // 获得Shapes的BoundingBox，赋值到box
    if (fabs(exp) > 0.02 ) { box.w += 2 * exp; box.h += 2 * exp; box.x -= exp; box.y -= exp; }
    
    auto s = al->CreateRectangle2(box.x, box.y, box.w, box.h, ZERO_4PC); // 使用BoundingBox box 创建一个矩形
    s->Outline->Color->RGBAssign(0, 255, 0);
    srs->Add(s);
  }
  srs->CreateSelection();
  
  sprintf(infobuf, "提示: 标记画框数量: %d 个\n容差值请使用小键盘输入", srs->Count);
  EndOpt(cdr);
  return true;
}

bool AutoMakeSelection(corel *cdr) {
  auto sr = cdr->ActiveSelectionRange; 
  if (0 == sr->Count) {
    auto all = cdr->ActiveDocument->ActivePage->Shapes->All();
    all->CreateSelection();
  }
  return true;
}

// 快速分组重叠的区域, 使用算法"Union-Find" 算法。这个算法可以有效地处理这种并集问题。
// 算法的时间复杂度为 O(n^2),其中 n 是矩形的数量。如果矩形数量较多,可以考虑使用更高效的算法,
// 例如使用四叉树(Quadtree)或者区间树(Interval Tree)等数据结构来加速计算。
bool Box_AutoGroup(corel *cdr, double exp) {
  BoundingBox box;

  auto sr = cdr->ActiveSelectionRange; // 获得选择范围
  auto al = cdr->ActiveLayer;          // 获得当前层
  if (!sr || !al) return false;

  auto start = std::chrono::high_resolution_clock::now(); // 开始时间
  BeginOpt(cdr);

  std::vector<BoundingBox> boxes;
  std::vector<int> parent;

  // CorelDRAW Shapes 物件 Item 编号从1开始
  for (auto i = 0; i != sr->Count; i++) {
    sr->Shapes->Item[i + 1]->GET_BOUNDING_BOX(box);
    boxes.push_back(box);
    parent.push_back(i);
  }

  // 扩展边界框，或者收缩边界框
  if (fabs(exp) > 0.02 ) {
    expand_bounding_boxes(boxes, exp);
  }

  // 实现 Union-Find 算法来合并重叠的区域
  for (int i = 0; i < boxes.size(); i++) {
    for (int j = i + 1; j < boxes.size(); j++) {
      if (isOverlapped(boxes[i], boxes[j])) {
        unionSet(parent, i, j);
      }
    }
  }

  double runtime[2] = {0,0};
  auto end = std::chrono::high_resolution_clock::now(); 
  std::chrono::duration<double> duration = end - start;
  runtime[0] = duration.count();

 // 输出分组结果到文件
  // std::ofstream output_file("D:\\group.txt");
  // if (output_file.is_open()) {
  //     std::map<int, std::vector<int>> groups;
  //     for (int i = 0; i < parent.size(); i++) {
  //         int root = find(parent, i);
  //         groups[root].push_back(i + 1); // CorelDRAW Shapes 物件 Item 编号从1开始
  //     }

  //     for (const auto& group : groups) {
  //         output_file << "Group: ";
  //         for (int index : group.second) {
  //             output_file << index << " ";
  //         }
  //         output_file << std::endl;
  //     }

  //   auto end = std::chrono::high_resolution_clock::now(); // 结束时间
  //   // 计算持续时间
  //   std::chrono::duration<double> duration = end - start;
  //   output_file << "Execution time: " << duration.count() << " seconds\n";

  //   output_file.close();
  // } 

  // 输出分组结果
  std::map<int, std::vector<int>> groups;
  for (int i = 0; i < parent.size(); i++) {
    int root = find(parent, i);
    groups[root].push_back(i + 1); // CorelDRAW Shapes 物件 Item 编号从1开始
  }

  auto srgp = cdr->CreateShapeRange();
  auto srs = cdr->CreateShapeRange();

  cdr->ActiveDocument->ClearSelection();
// 原来 没有取消选择 最初速度
// Execution time: 63.0305 seconds

// srgp->GET_BOUNDING_BOX(box); 
// al->CreateRectangle2(box.x, box.y, box.w, box.h, ZERO_4PC); // 使用边界 创建一个矩形
// box边界 转左上和右下坐标 box.x, box.y + box.h, box.x + box.w, box.y
// auto sh = cdr->ActivePage->SelectShapesFromRectangle(box.x, box.y + box.h, box.x + box.w, box.y, false);
// sh->Group();
// 使用 SelectShapesFromRectangle 框选的形状进行群组
// Execution time: 2.44753 seconds

// cdr->ActiveDocument->ClearSelection(); // 使用取消选择
// Execution time: 1.7432 seconds

// srgp->CreateSelection();
// cdr->ActiveSelectionRange->Group();
// Execution time: 1.87662 seconds

  // 分组分别进行群组
  for (const auto& group : groups) {
      for (int index : group.second) 
        srgp->Add(sr->Shapes->Item[index]);
      
      if(sr->Count >1)
        srs->Add(srgp->Group());
      else
        srs->AddRange(srgp);  

      srgp->RemoveAll();
  }
  srs->CreateSelection();

  // 计算持续时间
  duration = std::chrono::high_resolution_clock::now() - start;
  runtime[1] = duration.count();

  sprintf(infobuf, "选择物件: %d 个, 分组: %.2f秒\n总共群组: %d 组, 总时间: %.2f秒", sr->Count, runtime[0] + 0.01, srs->Count, runtime[1] + 0.02);
  EndOpt(cdr);
  return true;
}