#include "cdrapp.h"
#include <algorithm>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>

#define GET_BOUNDING_BOX(box)                                                  \
  GetBoundingBox(&(box).x, &(box).y, &(box).w, &(box).h, false)

#define ZERO_4PC 0, 0, 0, 0

typedef struct {
  double x; // 左下角 x 坐标
  double y; // 左下角 y 坐标
  double w; // 宽度
  double h; // 高度
} BoundingBox;

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

bool BBox_DrawRectangle(corel *cdr) {
  BoundingBox box;
  auto sr = cdr->ActiveSelectionRange; // 获得选择范围
  auto al = cdr->ActiveLayer;          // 获得当前层
  if (!sr || !al) return false;

  // CorelDRAW Shapes 物件 Item 编号从1开始
  for (auto i = 0; i != sr->Count; i++) {
    sr->Shapes->Item[i + 1]->GET_BOUNDING_BOX(box); // 获得Shapes的BoundingBox，赋值到box
    al->CreateRectangle2(box.x, box.y, box.w, box.h, ZERO_4PC); // 使用BoundingBox box 创建一个矩形
  }
  return true;
}


#include <thread>
#include <mutex>

std::mutex mtx; // 用于保护共享资源

void groupShapes(std::vector<int> shapes, IVGShapeRangePtr srgp, IVGShapeRangePtr sr) {
    for (int index : shapes) {
        srgp->Add(sr->Shapes->Item[index]);
    }
    if (shapes.size() > 1) {
        // 这里需要锁定，以防止多线程访问冲突
        mtx.lock();
        srgp->Group();
        mtx.unlock();
    }
    srgp->RemoveAll();
}

void processGroups(std::map<int, std::vector<int>> groups, corel* cdr , IVGShapeRangePtr sr) {
    std::vector<std::thread> threads;

    for (const auto& group : groups) {

        auto srgp = cdr->CreateShapeRange();

        // 启动线程处理每个分组
        threads.emplace_back(groupShapes, group.second, srgp, sr);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}


// 快速分组重叠的区域, 使用算法"Union-Find" 算法。这个算法可以有效地处理这种并集问题。
// 算法的时间复杂度为 O(n^2),其中 n 是矩形的数量。如果矩形数量较多,可以考虑使用更高效的算法,
// 例如使用四叉树(Quadtree)或者区间树(Interval Tree)等数据结构来加速计算。
bool Box_AutoGroup(corel *cdr) {
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

  // 实现 Union-Find 算法来合并重叠的区域
  for (int i = 0; i < boxes.size(); i++) {
    for (int j = i + 1; j < boxes.size(); j++) {
      if (isOverlapped(boxes[i], boxes[j])) {
        unionSet(parent, i, j);
      }
    }
  }

 // 输出分组结果到文件
  std::ofstream output_file("D:\\group.txt");
  if (output_file.is_open()) {
      std::map<int, std::vector<int>> groups;
      for (int i = 0; i < parent.size(); i++) {
          int root = find(parent, i);
          groups[root].push_back(i + 1); // CorelDRAW Shapes 物件 Item 编号从1开始
      }

      for (const auto& group : groups) {
          output_file << "Group: ";
          for (int index : group.second) {
              output_file << index << " ";
          }
          output_file << std::endl;
      }

    auto end = std::chrono::high_resolution_clock::now(); // 结束时间
    // 计算持续时间
    std::chrono::duration<double> duration = end - start;
    output_file << "Execution time: " << duration.count() << " seconds\n";

    output_file.close();
  } 

  // 输出分组结果
  std::map<int, std::vector<int>> groups;
  for (int i = 0; i < parent.size(); i++) {
    int root = find(parent, i);
    groups[root].push_back(i + 1); // CorelDRAW Shapes 物件 Item 编号从1开始
  }

 // 多线程调用
  processGroups(groups, cdr, sr);

// https://community.coreldraw.com/sdk/f/coreldraw-and-corel-designer-api/67436/i-am-using-c-to-write-a-cpg-plugin-when-using-shaperange-group-in-coreldraw-to-group-2-000-objects-it-takes-30-seconds
// 我在使用 C++ 编写CPG 插件，上万个物件重叠分组，C++ 算法速度很快，不到 0.25秒。但是使用 ShapeRange.Group在 CorelDRAW，群组2000个组，速度要30秒。怎么解决软件的瓶颈
// 我已经使用过窗口刷新优化，调用以上优化函数。现在瓶颈是 ShapeRange.Group 遍历执行2000次，CorelDRAW卡在哪里。是否有能通过多线程，或者SDK调用类似API解决次问题？
// 多线程调用 ShapeRange.Group() 进行批量群组，好的结果是是程序没有崩溃。坏的结果的并没有提高速度

  EndOpt(cdr);
  return true;
}