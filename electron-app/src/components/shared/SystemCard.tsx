import { ReactNode } from 'react';

interface SystemCardProps {
  title: string;
  icon: string;
  children: ReactNode;
}

const SystemCard = ({ title, icon, children }: SystemCardProps) => {
  return (
    <div className="glass rounded-3xl p-8 shadow-xl hover:shadow-2xl transition-all duration-300 animate-fade-in-up">
      <h3 className="text-xl font-bold text-gray-900 mb-6 flex items-center gap-2">
        <span>{icon}</span>
        {title}
      </h3>
      {children}
    </div>
  );
};

export default SystemCard;
