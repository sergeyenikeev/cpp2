#include <iostream>
// сделать реверс односвязного списка
struct ListNode
{
    ListNode* next;
    int value;
    ListNode() : value(0), next(nullptr) {};
    ListNode(int val) : value(val), next(nullptr) {};
    ListNode(int val, ListNode* nextNode) : value(val), next(nextNode) {};
};

void printList(ListNode* head) {
    ListNode* current = head;
    while (current != nullptr) {
        std::cout << current->value << " ";
        current = current->next;
    }
    std::cout << std::endl;
}

ListNode* reverseList(ListNode* head) {
    ListNode* prev = nullptr;
    ListNode* current = head;
    while (current != nullptr) {
        ListNode* nextNode = current->next; // Сохраняем следующий узел
        current->next = prev; // Разворачиваем ссылку
        prev = current; // Двигаем prev вперед
        current = nextNode; // Двигаем current вперед
    }
    return prev; // Новый голова списка
}

int main() {
    std::cout << "Test!!! Start" << '\n';
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    head->next->next = new ListNode(3);
    printList(head);

    ListNode* head2 = reverseList(head);
    printList(head2);
    std::cout << "Test!!! End" << '\n';
    return 0;
}